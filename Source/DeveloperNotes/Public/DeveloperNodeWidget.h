////nightfall16 @2025 , simple plugin to help sharing development notes with working team in the same project

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class ADeveloperNoteActor;

/**
 * widget to show and edit the notes
 */
class DEVELOPERNOTES_API SDeveloperNodeWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SDeveloperNodeWidget) {}
	SLATE_ARGUMENT(TWeakObjectPtr<ADeveloperNoteActor>, NoteActor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TWeakObjectPtr<ADeveloperNoteActor> NoteActor;

	TSharedPtr<class SEditableTextBox> TitleBox;
	TSharedPtr<class SMultiLineEditableTextBox> NoteBox;
	TSharedPtr<class SComboBox<TSharedPtr<FString>>> MentionBox;

	TSharedPtr<FString> CurrentValue;
	TArray<TSharedPtr<FString>> MentionsOptions;

	bool OpenedByMentionedOwner = false;

	void OnTitleCommitted(const FText& NewText, ETextCommit::Type CommitType);
	void OnNoteCommitted(const FText& NewText, ETextCommit::Type CommitType);

	void OnTitleChanged(const FText& NewText);
	void OnNoteChanged(const FText& NewText);

	void OnMentionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	bool bHasNoteChanged = false;
};