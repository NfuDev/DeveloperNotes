//nightfall16 @2025 , simple plugin to help sharing development notes with working team in the same project

#include "DeveloperNotes.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "IImageWrapperModule.h"
#include "LevelEditor.h"
#include "Selection.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "LevelEditorViewport.h"
#include "UObject/ObjectSaveContext.h"
#include "EngineUtils.h"


#include "DeveloperNoteActor.h"
#include "DeveloperNodeWidget.h"

UTexture2D* FDeveloperNotesModule::NotesIcon = nullptr;
UTexture2D* FDeveloperNotesModule::BugsIcon = nullptr;

#define LOCTEXT_NAMESPACE "FDeveloperNotesModule"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FDeveloperNotesEditorStyle::StyleInstance = nullptr;


void FDeveloperNotesEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FDeveloperNotesEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FDeveloperNotesEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("FDeveloperNotesEditorStyle"));
	return StyleSetName;
}


TSharedRef< FSlateStyleSet > FDeveloperNotesEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("FDeveloperNotesEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("DeveloperNotes")->GetBaseDir() / TEXT("Resources"));

	Style->Set("Note", new IMAGE_BRUSH(TEXT("NoteIcon"), FVector2D(20.0f, 20.0f)));

	return Style;
}

void FDeveloperNotesEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FDeveloperNotesEditorStyle::Get()
{
	return *StyleInstance;
}

UTexture2D* FDeveloperNotesEditorStyle::GetBrushAsTexture(FName BrushName)
{
	if (!StyleInstance) return nullptr;

	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("DeveloperNotes");
	if (!Plugin.IsValid())
		return nullptr;

	FString PluginResourcesDir = Plugin->GetBaseDir() / TEXT("Resources");
	FString FullFilePath = PluginResourcesDir / *BrushName.ToString();

    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullFilePath))
    {
        return nullptr;
    }

    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FullFilePath))
    {
        return nullptr;
    }

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

    if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
    {
        return nullptr;
    }

    TArray<uint8> UncompressedRGBA;
    if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
    {
        return nullptr;
    }

    const int32 Width = ImageWrapper->GetWidth();
    const int32 Height = ImageWrapper->GetHeight();
    if (Width <= 0 || Height <= 0)
        return nullptr;

    UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
    if (!NewTexture)
        return nullptr;

    void* TextureData = NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    const int32 DataSize = UncompressedRGBA.Num();
    FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), DataSize);
    NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

    NewTexture->SRGB = true;
    NewTexture->UpdateResource();

    return NewTexture;
}


void FDeveloperNotesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FDeveloperNotesEditorStyle::Initialize();
	FDeveloperNotesEditorStyle::ReloadTextures();


    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FDeveloperNotesModule::RegisterMenus));

    if (GUnrealEd)
    {
        TSharedPtr<FDeveloperNoteVisualizer> DeveloperNoteVisualizer = MakeShared<FDeveloperNoteVisualizer>();

        GUnrealEd->RegisterComponentVisualizer(UNoteComponent::StaticClass()->GetFName(), DeveloperNoteVisualizer);
        DeveloperNoteVisualizer->OnRegister();
    }


    NotesIcon = FDeveloperNotesEditorStyle::GetBrushAsTexture("NoteIcon.png");
    BugsIcon = FDeveloperNotesEditorStyle::GetBrushAsTexture("BugsIcon.png");
    NotesIcon->AddToRoot();
    BugsIcon->AddToRoot();

    FEditorDelegates::PostSaveWorldWithContext.AddRaw(this, &FDeveloperNotesModule::HandlePostWorldSave);
}

void FDeveloperNotesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FDeveloperNotesEditorStyle::Shutdown();

    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);

    if (GUnrealEd)
        GUnrealEd->UnregisterComponentVisualizer(UNoteComponent::StaticClass()->GetFName());
   
}

void FDeveloperNotesModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.ActorContextMenu");
    if (!Menu) return;

#if ENGINE_MINOR_VERSION == 3
    FToolMenuSection& Section = Menu->FindOrAddSection("DeveloperNotes");
    Section.InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::First);
#else
    FToolMenuSection& Section = Menu->FindOrAddSection("DeveloperNotes", FText::GetEmpty(), FToolMenuInsert(NAME_None, EToolMenuInsertType::First));
#endif

    Section.AddMenuEntry("DeveloperNotes", LOCTEXT("AddNoteLabel", "Add Developer Note"), LOCTEXT("AddNoteTooltip", "Spawns a new developer note"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"),
        FUIAction(FExecuteAction::CreateRaw(this, &FDeveloperNotesModule::SpawnNoteActorAtMouseLocation)));
    

    Menu->AddDynamicSection("DeveloperNoteActions", FNewToolMenuDelegate::CreateLambda([this](UToolMenu* InMenu)
        {
            USelection* SelectedActors = GEditor->GetSelectedActors();
            int32 NoteActorCount = 0;
            ADeveloperNoteActor* SelectedNote = nullptr;

            for (FSelectionIterator It(*SelectedActors); It; ++It)
            {
                if (ADeveloperNoteActor* Actor = Cast<ADeveloperNoteActor>(*It))
                {
                    SelectedNote = Actor;
                    NoteActorCount++;
                }
            }

            if (NoteActorCount == 1 && SelectedNote)
            {

#if ENGINE_MINOR_VERSION == 3
                FToolMenuSection& PreviewSection = InMenu->FindOrAddSection("DeveloperNotes");
                PreviewSection.InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::First);
#else

                FToolMenuSection& PreviewSection = InMenu->FindOrAddSection(
                    "DeveloperNotes",
                    FText::GetEmpty(),
                    FToolMenuInsert(NAME_None, EToolMenuInsertType::First)
                );
#endif


                PreviewSection.AddMenuEntry("ViewNoteAction", LOCTEXT("ViewNoteLabel", "View Note"), LOCTEXT("ViewNoteTooltip", "Opens the note overlay."),
                    FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Edit"),
                    FUIAction(FExecuteAction::CreateRaw(this, &FDeveloperNotesModule::OpenNoteWidget, SelectedNote)));
            }
        }));

}

void FDeveloperNotesModule::SpawnNoteActorAtMouseLocation()
{
    FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
    TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();

    if (!LevelEditor.IsValid()) return;

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;

    FVector SpawnLocation = FVector::ZeroVector;

    if (GCurrentLevelEditingViewportClient)
    {
        FViewportCursorLocation CursorLoc = GCurrentLevelEditingViewportClient->GetCursorWorldLocationFromMousePos();
        SpawnLocation = CursorLoc.GetOrigin();

        FVector RayOrigin = CursorLoc.GetOrigin();
        FVector RayDir = CursorLoc.GetDirection();
        SpawnLocation = RayOrigin + (RayDir * 1000.0f);
    }

    const FScopedTransaction Transaction(LOCTEXT("SpawnDevNote", "Spawn Developer Note"));

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.bCreateActorPackage = true;
    SpawnParams.ObjectFlags |= RF_Transactional;

    World->Modify();

    ADeveloperNoteActor* NewNote = World->SpawnActor<ADeveloperNoteActor>(ADeveloperNoteActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

    World->MarkPackageDirty();

    GEditor->SelectNone(true, true);
    GEditor->SelectActor(NewNote, true, true);
}

void FDeveloperNotesModule::OpenNoteWidget(ADeveloperNoteActor* NoteActor)
{
    if (!NoteActor) return;

    TSharedRef<SWindow> NoteWindow = SNew(SWindow)
        .Title(LOCTEXT("NoteDetails", "Developer Note"))
        .ClientSize(FVector2D(400, 300))
        .SupportsMaximize(false)
        .SupportsMinimize(false)
        .IsTopmostWindow(true)
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Fill)
                [
                    SNew(SDeveloperNodeWidget).NoteActor(NoteActor)
                ]
        ];

    FSlateApplication::Get().AddWindow(NoteWindow);
}

void FDeveloperNotesModule::HandlePostWorldSave(UWorld* World, FObjectPostSaveContext ObjectSaveContext)
{
    if (!World) return;

    for (TActorIterator<ADeveloperNoteActor> It(World); It; ++It)
    {
        ADeveloperNoteActor* Actor = *It;

        if (!Actor)continue;
        Actor->CheckForMention();
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDeveloperNotesModule, DeveloperNotes)

