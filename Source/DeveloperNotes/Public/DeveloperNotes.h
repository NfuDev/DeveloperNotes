//nightfall16 @2025 , simple plugin to help sharing development notes with working team in the same project

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "DeveloperNodeWidget.h"
#include "Styling/SlateStyle.h"
#include "NotesDeveloperSettings.h"

class FDeveloperNotesEditorStyle
{
public:

	static void Initialize();

	static void Shutdown();

	static void ReloadTextures();

	static const ISlateStyle& Get();

	static UTexture2D* GetBrushAsTexture(FName BrushName);

	static FName GetStyleSetName();

private:

	static TSharedRef< class FSlateStyleSet > Create();

private:

	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};

class FDeveloperNotesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RegisterMenus();
	void OpenNoteWidget(class ADeveloperNoteActor* NoteActor);
	void SpawnNoteActorAtMouseLocation();
	
	void HandlePostWorldSave(UWorld* World, FObjectPostSaveContext ObjectSaveContext);

	static UTexture2D* NotesIcon;
	static UTexture2D* BugsIcon;

	inline static  UTexture2D* GetNoteTexture(ENoteType NoteType)
	{
		return NoteType == ENoteType::Note ? NotesIcon : BugsIcon;
	}
};
