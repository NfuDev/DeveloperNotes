//nightfall16 @2025 , simple plugin to help sharing development notes with working team in the same project

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NotesDeveloperSettings.generated.h"

UENUM(BlueprintType)
enum class ENoteType : uint8
{
	Note,
	Bug,
};


UCLASS(Config = DeveloperNotes, DefaultConfig)
class DEVELOPERNOTES_API UDeveloperNotesSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDeveloperNotesSettings();

	//the name used to sign your notes for other devs to know and also to get notifications about other devs menitioning you to certain notes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (GetOptions = "DevNames"))
	FName DeveloperName;

	FString UserSaveFile = "DeveloperName.json";

	//all team developers.
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TArray<FString> TeamDevelopers;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FColor NoteTextColor = FColor::White;

	UFUNCTION()
	FORCEINLINE TArray<FString> DevNames()
	{
		return TeamDevelopers;
	}

	virtual void PostEditChangeProperty(FPropertyChangedEvent& changeEvent) override;
};