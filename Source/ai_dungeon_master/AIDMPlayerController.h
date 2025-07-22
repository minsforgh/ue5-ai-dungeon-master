// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "InputActionValue.h"
#include "AIDMPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UChatWidget;
class AAIManager;

/**
 * AI Dungeon Master Player Controller
 * Manages UI and chat system
 */
UCLASS(abstract)
class AI_DUNGEON_MASTER_API AAIDMPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Chat Widget Class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UChatWidget> ChatWidgetClass;

	/** Chat Widget Instance */
	UPROPERTY()
	UChatWidget* ChatWidget;

	/** Chat Widget Visibility State */
	bool bIsChatVisible = false;

	/** AI Manager Reference */
	UPROPERTY()
	AAIManager* AIManager;

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Enhanced Input - AI Dungeon Master Context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* AIDungeonMasterMappingContext;

	/** Enhanced Input - Toggle Chat Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	UInputAction* ToggleChatAction;

	/** Enhanced Input - Close Chat Action (ESC) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	UInputAction* CloseChatAction;

	// Chat Widget Functions
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ShowChatWidget();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void HideChatWidget();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ToggleChatWidget();

	// Console Commands for testing
	UFUNCTION(Exec)
	void ShowChat();

	UFUNCTION(Exec)
	void HideChat();

	UFUNCTION(Exec)
	void ToggleChat();

protected:
	// Enhanced Input Handlers
	void OnToggleChatTriggered(const FInputActionValue& Value);
	void OnCloseChatTriggered(const FInputActionValue& Value);

	// AI Integration
	void SetupAIManager();

	UFUNCTION()
	void OnUserMessageSent(const FString& Message);

	UFUNCTION()
	void OnAIResponseReceived(bool bSuccess, const FString& Response);

	// 강제 줄바꿈 추가
	FString FormatMessageWithLineBreaks(const FString& Message);
};