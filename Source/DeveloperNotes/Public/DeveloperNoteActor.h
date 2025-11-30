//nightfall16 @2025 , simple plugin to help sharing development notes with working team in the same project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ComponentVisualizer.h"
#include "NotesDeveloperSettings.h"
#include "DeveloperNoteActor.generated.h"

class FDeveloperNoteVisualizer : public FComponentVisualizer
{
public:
	FDeveloperNoteVisualizer();
	virtual ~FDeveloperNoteVisualizer() override;

	// FComponentVisualizer interface
	virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
};

UCLASS()
class DEVELOPERNOTES_API UNoteComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNoteComponent();

};

UCLASS()
class DEVELOPERNOTES_API ADeveloperNoteActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADeveloperNoteActor();

	UPROPERTY()
	FText Title;


	UPROPERTY()
	FText Note;


	UPROPERTY()
	FText Author;

	UPROPERTY()
	FText MentionedDeveloper;

	UPROPERTY()
	bool bDeveloperRead = false;

	UPROPERTY()
	FString ModifiedBy;

	UPROPERTY()
	ENoteType NoteType;

	UPROPERTY()
	TObjectPtr<UBillboardComponent> SpriteComponent;

	UPROPERTY()
	TObjectPtr<UNoteComponent> NoteComponent;


	virtual void PostLoad() override;
	void CheckForMention();
	void UpdateNoteIcon();
};
