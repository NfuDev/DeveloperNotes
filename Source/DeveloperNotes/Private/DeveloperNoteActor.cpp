//nightfall16 @2025 , simple plugin to help sharing development notes with working team in the same project


#include "DeveloperNoteActor.h"
#include "Components/BillboardComponent.h"
#include "DeveloperNotes.h"
#include "Engine/Canvas.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

ADeveloperNoteActor::ADeveloperNoteActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
    SetActorHiddenInGame(true);
    bIsEditorOnlyActor = true;

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Note"));
	SpriteComponent->SetupAttachment(RootComponent);
	RootComponent = SpriteComponent;

    NoteComponent = CreateDefaultSubobject<UNoteComponent>(TEXT("NoteComponent"));
  
    UpdateNoteIcon();

	Title = FText::FromString(TEXT("New Note"));
	Note = FText::FromString(TEXT("Write your note here..."));
	Author = FText::FromName(GetMutableDefault<UDeveloperNotesSettings>()->DeveloperName);

}

void ADeveloperNoteActor::CheckForMention()
{
    if (!MentionedDeveloper.IsEmpty() && !bDeveloperRead)
    {
        if(MentionedDeveloper.EqualTo(FText::FromName(GetMutableDefault<UDeveloperNotesSettings>()->DeveloperName)))
        {
            FNotificationInfo Info(FText::FromString(FString::Printf(TEXT("The developer %s mentioned you in [ %s ] note"), *Author.ToString(), *Title.ToString())));

            Info.bUseSuccessFailIcons = true;
            Info.bFireAndForget = true;
            Info.ExpireDuration = 10.0f;

            Info.HyperlinkText = FText::FromString(FString::Printf(TEXT("Click to inspect %s Note"), *Title.ToString()));
            Info.Hyperlink = FSimpleDelegate::CreateLambda([this]()
                {
                    if (GIsEditor && this && this->IsValidLowLevel())
                    {
                        GEditor->SelectNone(false, true, false);
                        GEditor->SelectActor(this, true, true, true);

                        GEditor->Exec(this->GetWorld(), TEXT("CAMERA ALIGN"));
                    }
                });

            FSlateNotificationManager::Get().AddNotification(Info);
        }
    }
}

void ADeveloperNoteActor::UpdateNoteIcon()
{
    UTexture2D* NoteSprite = FDeveloperNotesModule::GetNoteTexture(NoteType);

    if (NoteSprite && NoteSprite->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Warning, TEXT("Texture must be loaded for note"));
        SpriteComponent->SetSprite(NoteSprite);
    }

    else
        UE_LOG(LogTemp, Warning, TEXT("Developer note actor couldn't load note texture"));
}

void ADeveloperNoteActor::PostLoad()
{
    Super::PostLoad();

    if (SpriteComponent && SpriteComponent->Sprite == nullptr)
    {
        SpriteComponent->SetSprite(FDeveloperNotesModule::GetNoteTexture(NoteType));
    }

    CheckForMention();
}

FDeveloperNoteVisualizer::FDeveloperNoteVisualizer()
{
}

FDeveloperNoteVisualizer::~FDeveloperNoteVisualizer()
{
}

void FDeveloperNoteVisualizer::DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
    if (!Component || !Component->GetOwner()) return;

    const ADeveloperNoteActor* NoteActor = Cast<ADeveloperNoteActor>(Component->GetOwner());
    if (!NoteActor) return;

    if (!Canvas) return;

    FVector WorldLocation = NoteActor->GetActorLocation();

    WorldLocation.Z += 190.0f;

    FVector2D ScreenLocation;
    bool bOnScreen = View->WorldToPixel(WorldLocation, ScreenLocation);

    if (bOnScreen)
    {
        FString NoteText = FString::Printf(TEXT("Note: %s\nAuthor: %s"),*NoteActor->Title.ToString(), *NoteActor->Author.ToString());
            
        UFont* Font = GEngine->GetSmallFont();
        float TextX = ScreenLocation.X;
        float TextY = ScreenLocation.Y;

        Canvas->DrawShadowedString(TextX, TextY, *NoteText, Font, GetMutableDefault<UDeveloperNotesSettings>()->NoteTextColor); 

    }
}

void FDeveloperNoteVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    if (!Component || !Component->GetOwner()) return;

    const ADeveloperNoteActor* NoteActor = Cast<ADeveloperNoteActor>(Component->GetOwner());
    if (!NoteActor) return;
;
    for (AActor* contextActor : NoteActor->NoteContext)
    {
        PDI->DrawLine(NoteActor->GetActorLocation(), contextActor->GetActorLocation(), FColor::Yellow, ESceneDepthPriorityGroup::SDPG_Foreground, 15.0f);
        PDI->DrawPoint(contextActor->GetActorLocation(), FColor::Yellow, 20.0f, ESceneDepthPriorityGroup::SDPG_Foreground);
    }

}

UNoteComponent::UNoteComponent()
{
}
