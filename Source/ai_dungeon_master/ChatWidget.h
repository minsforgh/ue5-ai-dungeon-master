// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ChatWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageSent, const FString&, Message);

/**
 * Chat UI Widget for AI Dungeon Master
 * Handles user input and displays chat history
 */
UCLASS()
class AI_DUNGEON_MASTER_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UChatWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// Widget components - bound in Blueprint
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* MessageInputBox;

	UPROPERTY(meta = (BindWidget))
	class UButton* SendButton;

	// Chat message template
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat")
	TSubclassOf<class UUserWidget> ChatMessageWidgetClass;

public:
	// Events
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnMessageSent OnMessageSent;

	// Public functions
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void AddUserMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void AddAIMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void AddSystemMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ClearChat();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void FocusInputBox();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Input handling
	UFUNCTION()
	void OnSendButtonClicked();

	UFUNCTION()
	void OnMessageInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// Helper functions
	void SendCurrentMessage();
	void AddChatMessage(const FString& Message, const FString& SenderName, const FLinearColor& Color);
	void ScrollToBottom();

private:
	// Chat history
	TArray<FString> ChatHistory;
	static const int32 MaxChatHistory = 100;
};