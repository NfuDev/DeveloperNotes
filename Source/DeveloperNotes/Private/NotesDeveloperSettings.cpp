//nightfall16 @2025 , simple plugin to help sharing development notes with working team in the same project

#include "NotesDeveloperSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

UDeveloperNotesSettings::UDeveloperNotesSettings()
{
	FString AppData = FPlatformProcess::UserSettingsDir();
	AppData += "UnrealDeveloperNotes";

	if(IFileManager::Get().DirectoryExists(*AppData))
	{
		UE_LOG(LogTemp, Warning, TEXT("directory was found already : %s"), *AppData);

		AppData.Append("/");
		AppData.Append(UserSaveFile);

		if(IFileManager::Get().FileExists(*AppData))
		{
			FString JsonFile;
			if(FFileHelper::LoadFileToString(JsonFile, *AppData))
			{
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonFile);

				if (FJsonSerializer::Deserialize(Reader, JsonObject))
				{
					UE_LOG(LogTemp, Warning, TEXT("developer name loaded from file : %s"), *AppData);

					FString Loadedname = JsonObject->GetStringField(TEXT("DeveloperName"));
					if(!Loadedname.IsEmpty())
					{
						DeveloperName = FName(Loadedname);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("developer note failed to load save file"));
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("developer note failed to load save file"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("developer note save file was not found, user didn't set the developer name"));
		}
	}

	if(DeveloperName.IsNone())
	{
		FNotificationInfo Info(FText::FromString("You don't have developer name set, please go to project settings->DeveloperNotesSettings and add your name to the list and also set your name so the notes get signed with your name and you can mention other devs"));
		Info.bUseSuccessFailIcons = true;
		Info.bFireAndForget = true;
		Info.ExpireDuration = 10.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

void UDeveloperNotesSettings::PostEditChangeProperty(FPropertyChangedEvent& changeEvent)
{
	if(changeEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UDeveloperNotesSettings, DeveloperName))
	{
		TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

		JsonObject->SetStringField(TEXT("DeveloperName"), DeveloperName.ToString());

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

		if(FJsonSerializer::Serialize(JsonObject, Writer))
		{
			FString AppData = FPlatformProcess::UserSettingsDir();
			AppData += "UnrealDeveloperNotes";

			if (IFileManager::Get().DirectoryExists(*AppData))
			{
				AppData.Append("/");
				AppData.Append(UserSaveFile);

				if (FFileHelper::SaveStringToFile(OutputString, *AppData))
				{
					UE_LOG(LogTemp, Warning, TEXT("developer name saved to file : %s"), *AppData);
				}
			}

			else
			{
				if (IFileManager::Get().MakeDirectory(*AppData, false))
				{
					AppData.Append("/");
					AppData.Append(UserSaveFile);

					if (FFileHelper::SaveStringToFile(OutputString, *AppData))
					{
						UE_LOG(LogTemp, Warning, TEXT("developer name saved to file : %s"), *AppData);
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to serialize developer name"));
		}
	}
}