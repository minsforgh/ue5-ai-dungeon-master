// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"

UChatWidget::UChatWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set default values
	ChatHistory.Empty();
}

void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind events
	if (SendButton)
	{
		SendButton->OnClicked.AddDynamic(this, &UChatWidget::OnSendButtonClicked);
	}

	if (MessageInputBox)
	{
		MessageInputBox->OnTextCommitted.AddDynamic(this, &UChatWidget::OnMessageInputCommitted);
	}

	// Focus input box
	FocusInputBox();

	// Add welcome message
	AddSystemMessage(TEXT("Welcome to AI Dungeon Master! Type your actions and let the AI guide your adventure."));
}

void UChatWidget::NativeDestruct()
{
	// Cleanup
	if (SendButton)
	{
		SendButton->OnClicked.RemoveAll(this);
	}

	if (MessageInputBox)
	{
		MessageInputBox->OnTextCommitted.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UChatWidget::OnSendButtonClicked()
{
	SendCurrentMessage();
}

void UChatWidget::OnMessageInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		SendCurrentMessage();
	}
}

void UChatWidget::SendCurrentMessage()
{
	if (!MessageInputBox)
	{
		return;
	}

	FString Message = MessageInputBox->GetText().ToString().TrimStartAndEnd();

	if (Message.IsEmpty())
	{
		return;
	}

	// Add user message to chat
	AddUserMessage(Message);

	// Clear input box
	MessageInputBox->SetText(FText::GetEmpty());

	// Broadcast message to listeners (AI Manager)
	OnMessageSent.Broadcast(Message);

	// Focus back to input
	FocusInputBox();
}

void UChatWidget::AddUserMessage(const FString& Message)
{
	AddChatMessage(Message, TEXT("You"), FLinearColor::Blue);

	ScrollToBottom();
}

void UChatWidget::AddAIMessage(const FString& Message)
{
	AddChatMessage(Message, TEXT("AI DM"), FLinearColor::Green);

	ScrollToBottom();
}

void UChatWidget::AddSystemMessage(const FString& Message)
{
	AddChatMessage(Message, TEXT("System"), FLinearColor::Gray);

	ScrollToBottom();
}

void UChatWidget::AddChatMessage(const FString& Message, const FString& SenderName, const FLinearColor& Color)
{
	if (!ChatScrollBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChatScrollBox is null"));
		return;
	}

	// Create message widget
	if (ChatMessageWidgetClass)
	{
		UUserWidget* MessageWidget = CreateWidget<UUserWidget>(this, ChatMessageWidgetClass);
		if (MessageWidget)
		{
			// Try to find TextBlock components in the message widget
			UTextBlock* SenderTextBlock = Cast<UTextBlock>(MessageWidget->GetWidgetFromName(TEXT("SenderText")));
			UTextBlock* MessageTextBlock = Cast<UTextBlock>(MessageWidget->GetWidgetFromName(TEXT("MessageText")));

			if (SenderTextBlock)
			{
				SenderTextBlock->SetText(FText::FromString(SenderName + TEXT(":")));
				SenderTextBlock->SetColorAndOpacity(FSlateColor(Color));
			}

			if (MessageTextBlock)
			{
				MessageTextBlock->SetText(FText::FromString(Message));
			}

			ChatScrollBox->AddChild(MessageWidget);
		}
	}
	else
	{
		// Fallback: Create simple text widget
		UTextBlock* TextBlock = NewObject<UTextBlock>(this);
		if (TextBlock)
		{
			FString FullMessage = FString::Printf(TEXT("[%s] %s"), *SenderName, *Message);
			TextBlock->SetText(FText::FromString(FullMessage));
			TextBlock->SetColorAndOpacity(FSlateColor(Color));
			ChatScrollBox->AddChild(TextBlock);
		}
	}

	// Add to history
	ChatHistory.Add(FString::Printf(TEXT("[%s] %s"), *SenderName, *Message));

	// Limit history size
	if (ChatHistory.Num() > MaxChatHistory)
	{
		ChatHistory.RemoveAt(0, ChatHistory.Num() - MaxChatHistory);
	}

	// Scroll to bottom
	ScrollToBottom();

	// Debug output
	UE_LOG(LogTemp, Log, TEXT("Chat: [%s] %s"), *SenderName, *Message);
}

void UChatWidget::ClearChat()
{
	if (ChatScrollBox)
	{
		ChatScrollBox->ClearChildren();
	}
	ChatHistory.Empty();
	AddSystemMessage(TEXT("Chat cleared."));
}

void UChatWidget::FocusInputBox()
{
	if (MessageInputBox)
	{
		MessageInputBox->SetKeyboardFocus();
	}
}

void UChatWidget::ScrollToBottom()
{
	if (ChatScrollBox)
	{
		// Delay scroll to ensure widget is rendered
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				if (ChatScrollBox)
				{
					ChatScrollBox->ScrollToEnd();
				}
			});
	}
}

