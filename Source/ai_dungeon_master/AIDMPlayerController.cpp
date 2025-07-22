// Copyright Epic Games, Inc. All Rights Reserved.
#include "AIDMPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "ChatWidget.h"
#include "AIManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void AAIDMPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// AI Manager ����
	SetupAIManager();
}

void AAIDMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Enhanced Input Subsystem�� Mapping Context �߰�
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// ���� Default Mapping Contexts �߰�
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}

		// AI Dungeon Master Mapping Context �߰�
		if (AIDungeonMasterMappingContext)
		{
			Subsystem->AddMappingContext(AIDungeonMasterMappingContext, 1);
		}
	}

	// Enhanced Input Component�� �׼� ���ε�
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Toggle Chat Action ���ε�
		if (ToggleChatAction)
		{
			EnhancedInputComponent->BindAction(ToggleChatAction, ETriggerEvent::Triggered, this, &AAIDMPlayerController::OnToggleChatTriggered);
		}
	}
}

void AAIDMPlayerController::ShowChatWidget()
{
	if (ChatWidgetClass && !ChatWidget)
	{
		// ChatWidget�� ������ ���� ����
		ChatWidget = CreateWidget<UChatWidget>(this, ChatWidgetClass);
		if (ChatWidget)
		{
			ChatWidget->AddToViewport();

			// �޽��� ���� �̺�Ʈ ���ε�
			ChatWidget->OnMessageSent.AddDynamic(this, &AAIDMPlayerController::OnUserMessageSent);

			// ȯ�� �޽��� ǥ��
			ChatWidget->AddSystemMessage(TEXT("Welcome to AI Dungeon Master!"));
			ChatWidget->AddSystemMessage(TEXT("Press T to open chat and start your adventure."));
		}
	}

	if (ChatWidget)
	{
		// Visibility �������� ǥ��
		ChatWidget->SetVisibility(ESlateVisibility::Visible);
		bIsChatVisible = true;

		// Enable mouse cursor and UI input
		SetShowMouseCursor(true);

		// GameAndUI ��忡���� ���� �Է��� �����ϵ��� ����
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(ChatWidget->TakeWidget());
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);

		// Focus on input box
		ChatWidget->FocusInputBox();
	}
}

void AAIDMPlayerController::HideChatWidget()
{
	if (ChatWidget)
	{
		// SetVisibility�� �����
		ChatWidget->SetVisibility(ESlateVisibility::Collapsed);
		bIsChatVisible = false;

		// Return to game mode
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
}

void AAIDMPlayerController::ToggleChatWidget()
{
	if (bIsChatVisible)
	{
		HideChatWidget();
	}
	else
	{
		ShowChatWidget();
	}
}

void AAIDMPlayerController::OnToggleChatTriggered(const FInputActionValue& Value)
{
	// ä�� ���� ���
	ToggleChatWidget();
}

void AAIDMPlayerController::OnCloseChatTriggered(const FInputActionValue& Value)
{
	// ESC ��� �߰��� ���
}

// Console Commands for testing
void AAIDMPlayerController::ShowChat()
{
	ShowChatWidget();
}

void AAIDMPlayerController::HideChat()
{
	HideChatWidget();
}

void AAIDMPlayerController::ToggleChat()
{
	ToggleChatWidget();
}

void AAIDMPlayerController::SetupAIManager()
{
	// ���忡�� AIManager ã��
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAIManager> ActorItr(World); ActorItr; ++ActorItr)
		{
			AIManager = *ActorItr;
			break;
		}

		if (!AIManager)
		{
			// AIManager�� ������ ���� ����
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = TEXT("AIManager");
			AIManager = World->SpawnActor<AAIManager>(AAIManager::StaticClass(), SpawnParams);
		}

		if (AIManager)
		{
			// AI ���� �̺�Ʈ ���ε�
			AIManager->OnAIResponse.AddDynamic(this, &AAIDMPlayerController::OnAIResponseReceived);
		}
	}
}

void AAIDMPlayerController::OnUserMessageSent(const FString& Message)
{
	// ����� �޽����� ChatWidget���� �̹� ǥ�õǹǷ� ���⼭�� ����

	// AI Manager���� �޽��� ����
	if (AIManager)
	{
		// �ε� �޽��� ǥ��
		if (ChatWidget)
		{
			ChatWidget->AddSystemMessage(TEXT("AI is preparing response..."));
		}

		AIManager->SendMessage(Message);
	}
	else
	{
		// AIManager�� ���� ��� ���� �޽���
		if (ChatWidget)
		{
			ChatWidget->AddSystemMessage(TEXT("Error: AI Manager not found."));
		}
	}
}

void AAIDMPlayerController::OnAIResponseReceived(bool bSuccess, const FString& Response)
{
	if (ChatWidget)
	{
		if (bSuccess)
		{
			// ���� �ٹٲ� ����
			FString FormattedResponse = FormatMessageWithLineBreaks(Response);
			ChatWidget->AddAIMessage(FormattedResponse);
		}
		else
		{
			ChatWidget->AddSystemMessage(TEXT("Error: Failed to get AI response."));
		}
	}
}

FString AAIDMPlayerController::FormatMessageWithLineBreaks(const FString& Message)
{
	const int32 MaxLineLength = 120; // ���� ����
	FString FormattedMessage;
	FString CurrentLine;

	// ���� ���� ������ �и� (. ! ? ����)
	TArray<FString> Sentences;
	FString TempMessage = Message;
	TempMessage = TempMessage.Replace(TEXT(". "), TEXT(".|||"));
	TempMessage = TempMessage.Replace(TEXT("! "), TEXT("!|||"));
	TempMessage = TempMessage.Replace(TEXT("? "), TEXT("?|||"));
	TempMessage = TempMessage.Replace(TEXT(".\n"), TEXT(".|||"));
	TempMessage = TempMessage.Replace(TEXT("!\n"), TEXT("!|||"));
	TempMessage = TempMessage.Replace(TEXT("?\n"), TEXT("?|||"));
	TempMessage.ParseIntoArray(Sentences, TEXT("|||"), true);

	for (const FString& Sentence : Sentences)
	{
		FString CleanSentence = Sentence.TrimStartAndEnd();
		if (CleanSentence.IsEmpty()) continue;

		// ������ ª���� �״�� �߰�
		if (CleanSentence.Len() <= MaxLineLength)
		{
			if (!CurrentLine.IsEmpty() && (CurrentLine + TEXT(" ") + CleanSentence).Len() > MaxLineLength)
			{
				// ���� ���� ���� ���� �� �ٷ�
				FormattedMessage += CurrentLine + TEXT("\n");
				CurrentLine = CleanSentence;
			}
			else
			{
				// ���� �ٿ� �߰�
				if (!CurrentLine.IsEmpty())
				{
					CurrentLine += TEXT(" ");
				}
				CurrentLine += CleanSentence;
			}
		}
		else
		{
			// �� ������ �ܾ�/���� ������ �и�
			if (!CurrentLine.IsEmpty())
			{
				FormattedMessage += CurrentLine + TEXT("\n");
				CurrentLine.Empty();
			}

			// �������� �ܾ� �и� �õ�
			TArray<FString> Words;
			CleanSentence.ParseIntoArray(Words, TEXT(" "), true);

			if (Words.Num() > 1)
			{
				// ����: �ܾ� ������ ó��
				for (const FString& Word : Words)
				{
					if ((CurrentLine + Word).Len() > MaxLineLength && !CurrentLine.IsEmpty())
					{
						FormattedMessage += CurrentLine + TEXT("\n");
						CurrentLine = Word + TEXT(" ");
					}
					else
					{
						CurrentLine += Word + TEXT(" ");
					}
				}
			}
			else
			{
				// �ѱ���: ���� ������ ���� �и�
				FString LongWord = CleanSentence;
				while (LongWord.Len() > MaxLineLength)
				{
					FormattedMessage += LongWord.Left(MaxLineLength) + TEXT("\n");
					LongWord = LongWord.Mid(MaxLineLength);
				}
				if (!LongWord.IsEmpty())
				{
					CurrentLine += LongWord;
				}
			}
		}
	}

	// ������ �� �߰�
	if (!CurrentLine.IsEmpty())
	{
		FormattedMessage += CurrentLine.TrimEnd();
	}

	return FormattedMessage;
}