////nightfall16 @2025 , simple plugin to help sharing development notes with working team in the same project


#include "DeveloperNodeWidget.h"
#include "DeveloperNoteActor.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Components/WidgetSwitcher.h"

void SDeveloperNodeWidget::Construct(const FArguments& InArgs)
{
	NoteActor = InArgs._NoteActor;


	ADeveloperNoteActor* Actor = NoteActor.Get();

	FText Title = Actor ? Actor->Title : FText::FromString(TEXT("note actor is not valid"));
	FText Note = Actor ? Actor->Note : FText::FromString(TEXT("note actor is not valid"));
	FText Author = Actor ? Actor->Author : FText::FromString(TEXT("note actor is not valid"));
	FText ModifiedBy = Actor ? FText::FromString(Actor->ModifiedBy) : FText::FromString(TEXT("note actor is not valid"));

	MentionsOptions.Add(MakeShared<FString>(FString("None")));

	OpenedByMentionedOwner = NoteActor->MentionedDeveloper.EqualTo(FText::FromName(GetMutableDefault<UDeveloperNotesSettings>()->DeveloperName));

	for (FString Developer : GetMutableDefault<UDeveloperNotesSettings>()->TeamDevelopers)
	{
		if (Developer == GetMutableDefault<UDeveloperNotesSettings>()->DeveloperName.ToString()) continue;

		TSharedPtr<FString> DveSP = MakeShared<FString>(Developer);
		MentionsOptions.Add(DveSP);

		if(Developer == Actor->MentionedDeveloper.ToString())
		{
			CurrentValue = DveSP;
		}
	}

	NoteTypeOptions.Add(MakeShared<FString>(FString("Note")));
	NoteTypeOptions.Add(MakeShared<FString>(FString("Bug ")));

	CurrentNoteType = NoteActor->NoteType == ENoteType::Note ? NoteTypeOptions[0] : NoteTypeOptions[1];

	ChildSlot
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(2)
						.AutoHeight()
						[
							SNew(STextBlock).Text(FText::FromString("Note Title : "))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(2)
						[
							SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.HAlign(HAlign_Fill)
								[
									SAssignNew(TitleBox, SEditableTextBox)
										.Text(Title)
										.OnTextCommitted(this, &SDeveloperNodeWidget::OnTitleCommitted)
										.OnTextChanged(this, &SDeveloperNodeWidget::OnTitleChanged)
								]
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SComboBox<TSharedPtr<FString>>)
										.OptionsSource(&NoteTypeOptions)
										.OnSelectionChanged(this, &SDeveloperNodeWidget::OnNoteTypeChanged)
										.OnGenerateWidget_Lambda([](const TSharedPtr<FString>& Item)
											{
												return SNew(STextBlock).Text(FText::FromString(*Item));
											})
										.InitiallySelectedItem(CurrentNoteType)
										[
											SNew(STextBlock)
												.Text_Lambda([this]()
													{
														return FText::FromString(CurrentNoteType.IsValid() ? *CurrentNoteType : TEXT("Note"));
													})
										]
								]
						]

						+ SVerticalBox::Slot().AutoHeight().Padding(2)
						.AutoHeight()
						[
							SNew(STextBlock).Text(FText::FromString("Note Details : "))
						]
				        + SVerticalBox::Slot().FillHeight(1.0f).Padding(2)
						[
							SAssignNew(NoteBox, SMultiLineEditableTextBox)
								.Text(Note)
								.OnTextCommitted(this, &SDeveloperNodeWidget::OnNoteCommitted)
								.OnTextChanged(this, &SDeveloperNodeWidget::OnNoteChanged)
								.AllowMultiLine(true)
								.ToolTipText(ModifiedBy)
						]

					    + SVerticalBox::Slot().AutoHeight().Padding(2)
						.AutoHeight()
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(STextBlock).Text(FText::FromString("Auther : "))
								]
								+ SHorizontalBox::Slot()
								.HAlign(HAlign_Fill)
								[
									SNew(STextBlock)
										.Text(Author)
								]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(2)
						.AutoHeight()
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock).Text(FText::FromString("Mention : "))
								]
								+ SHorizontalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Center)
								[
									SNew(SWidgetSwitcher)
										.WidgetIndex(OpenedByMentionedOwner)
										+SWidgetSwitcher::Slot()
										[
											SAssignNew(MentionBox, SComboBox<TSharedPtr<FString>>)
												.OptionsSource(&MentionsOptions)
												.OnSelectionChanged(this, &SDeveloperNodeWidget::OnMentionChanged)
												.OnGenerateWidget_Lambda([](const TSharedPtr<FString>& Item)
													{
														return SNew(STextBlock).Text(FText::FromString(*Item));
													})
												.InitiallySelectedItem(CurrentValue)
												[
													SNew(STextBlock)
														.Text_Lambda([this]()
															{
																return FText::FromString(CurrentValue.IsValid() ? *CurrentValue : TEXT("None"));
															})
												]
										]
									+SWidgetSwitcher::Slot()
										[
											SNew(STextBlock).Text(FText::FromString(TEXT("Auther Mentioned you")))
										]
								]
							    + SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
										.IsChecked(NoteActor->bDeveloperRead)
										[
											SNew(STextBlock).Text(FText::FromString("Is Resolved"))
										]
										.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckState)
											{
												NoteActor->Modify();
												NoteActor->bDeveloperRead = CheckState == ECheckBoxState::Checked;
												NoteActor->MarkPackageDirty();	
											})
								]
						]
				]
			];

}

void SDeveloperNodeWidget::OnTitleCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if(NoteActor.IsValid() && bHasNoteChanged)
	{
		NoteActor->Modify();
		NoteActor->Title = NewText;
		NoteActor->MarkPackageDirty();
	}
	bHasNoteChanged = false;
}

void SDeveloperNodeWidget::OnNoteCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (NoteActor.IsValid() && bHasNoteChanged)
	{
		NoteActor->Modify();
		NoteActor->Note = NewText;
		NoteActor->MarkPackageDirty();
	}

	bHasNoteChanged = false;
}

void SDeveloperNodeWidget::OnTitleChanged(const FText& NewText)
{
	if (NoteActor.IsValid())
	{
		NoteActor->Title = NewText;
	}

	if(!bHasNoteChanged)
	   bHasNoteChanged = true;
}

void SDeveloperNodeWidget::OnNoteChanged(const FText& NewText)
{
	if (!bHasNoteChanged)
		bHasNoteChanged = true;

	if(NoteActor.IsValid() && NoteActor->Author.ToString() != GetMutableDefault<UDeveloperNotesSettings>()->DeveloperName.ToString())
	{
		if(NoteActor->ModifiedBy.IsEmpty())
		{
			NoteActor->Modify();
			NoteActor->ModifiedBy = "Modified by :";
			NoteActor->ModifiedBy.Append(GetMutableDefault<UDeveloperNotesSettings>()->DeveloperName.ToString());
			NoteActor->MarkPackageDirty();
		}

		else
		{
		   NoteActor->Modify();
		   NoteActor->ModifiedBy.Append(FString::Printf(TEXT(", %s"), *GetMutableDefault<UDeveloperNotesSettings>()->DeveloperName.ToString()));
		   NoteActor->MarkPackageDirty();
		}
	}
}

void SDeveloperNodeWidget::OnMentionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if(NoteActor.IsValid() && NewSelection.IsValid())
	{
		FString SelectedOption = *NewSelection;
		NoteActor->Modify();
		NoteActor->MentionedDeveloper = FText::FromString(SelectedOption);
		NoteActor->MarkPackageDirty();
		
		CurrentValue = NewSelection;
	}
}

void SDeveloperNodeWidget::OnNoteTypeChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NoteActor.IsValid() && NewSelection.IsValid())
	{
		FString SelectedOption = *NewSelection;
		NoteActor->Modify();
		NoteActor->NoteType = SelectedOption == "Note" ? ENoteType::Note : ENoteType::Bug;
		NoteActor->UpdateNoteIcon();
		NoteActor->MarkPackageDirty();

		CurrentNoteType = NewSelection;
	}
}
