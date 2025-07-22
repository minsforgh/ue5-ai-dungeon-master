// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChatMessageWidget.h"
#include "Components/TextBlock.h"

UChatMessageWidget::UChatMessageWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UChatMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UChatMessageWidget::SetMessage(const FString& Sender, const FString& Message, const FLinearColor& SenderColor)
{
	if (SenderText)
	{
		SenderText->SetText(FText::FromString(Sender + TEXT(":")));
		SenderText->SetColorAndOpacity(FSlateColor(SenderColor));
	}

	if (MessageText)
	{
		MessageText->SetText(FText::FromString(Message));
	}
}