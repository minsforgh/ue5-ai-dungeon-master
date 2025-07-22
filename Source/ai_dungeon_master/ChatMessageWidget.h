// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ChatMessageWidget.generated.h"

/**
 * Individual chat message widget
 * Displays sender name and message content
 */
UCLASS()
class AI_DUNGEON_MASTER_API UChatMessageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UChatMessageWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// Widget components - bound in Blueprint
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SenderText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MessageText;

public:
	// Set message content
	UFUNCTION(BlueprintCallable, Category = "Chat Message")
	void SetMessage(const FString& Sender, const FString& Message, const FLinearColor& SenderColor = FLinearColor::White);

protected:
	virtual void NativeConstruct() override;
};