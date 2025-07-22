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

	// AI Manager 설정
	SetupAIManager();
}

void AAIDMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Enhanced Input Subsystem에 Mapping Context 추가
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// 기존 Default Mapping Contexts 추가
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}

		// AI Dungeon Master Mapping Context 추가
		if (AIDungeonMasterMappingContext)
		{
			Subsystem->AddMappingContext(AIDungeonMasterMappingContext, 1);
		}
	}

	// Enhanced Input Component에 액션 바인딩
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Toggle Chat Action 바인딩
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
		// ChatWidget이 없으면 새로 생성
		ChatWidget = CreateWidget<UChatWidget>(this, ChatWidgetClass);
		if (ChatWidget)
		{
			ChatWidget->AddToViewport();

			// 메시지 전송 이벤트 바인딩
			ChatWidget->OnMessageSent.AddDynamic(this, &AAIDMPlayerController::OnUserMessageSent);

			// 환영 메시지 표시
			ChatWidget->AddSystemMessage(TEXT("Welcome to AI Dungeon Master!"));
			ChatWidget->AddSystemMessage(TEXT("Press T to open chat and start your adventure."));
		}
	}

	if (ChatWidget)
	{
		// Visibility 변경으로 표시
		ChatWidget->SetVisibility(ESlateVisibility::Visible);
		bIsChatVisible = true;

		// Enable mouse cursor and UI input
		SetShowMouseCursor(true);

		// GameAndUI 모드에서도 게임 입력이 동작하도록 설정
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
		// SetVisibility로 숨기기
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
	// 채팅 위젯 토글
	ToggleChatWidget();
}

void AAIDMPlayerController::OnCloseChatTriggered(const FInputActionValue& Value)
{
	// ESC 기능 추가시 사용
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
	// 월드에서 AIManager 찾기
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAIManager> ActorItr(World); ActorItr; ++ActorItr)
		{
			AIManager = *ActorItr;
			break;
		}

		if (!AIManager)
		{
			// AIManager가 없으면 새로 생성
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = TEXT("AIManager");
			AIManager = World->SpawnActor<AAIManager>(AAIManager::StaticClass(), SpawnParams);
		}

		if (AIManager)
		{
			// AI 응답 이벤트 바인딩
			AIManager->OnAIResponse.AddDynamic(this, &AAIDMPlayerController::OnAIResponseReceived);
		}
	}
}

void AAIDMPlayerController::OnUserMessageSent(const FString& Message)
{
	// 사용자 메시지는 ChatWidget에서 이미 표시되므로 여기서는 생략

	// AI Manager에게 메시지 전송
	if (AIManager)
	{
		// 로딩 메시지 표시
		if (ChatWidget)
		{
			ChatWidget->AddSystemMessage(TEXT("AI is preparing response..."));
		}

		AIManager->SendMessage(Message);
	}
	else
	{
		// AIManager가 없을 경우 에러 메시지
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
			// 강제 줄바꿈 적용
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
	const int32 MaxLineLength = 120; // 단위 길이
	FString FormattedMessage;
	FString CurrentLine;

	// 먼저 문장 단위로 분리 (. ! ? 기준)
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

		// 문장이 짧으면 그대로 추가
		if (CleanSentence.Len() <= MaxLineLength)
		{
			if (!CurrentLine.IsEmpty() && (CurrentLine + TEXT(" ") + CleanSentence).Len() > MaxLineLength)
			{
				// 현재 줄이 가득 차면 새 줄로
				FormattedMessage += CurrentLine + TEXT("\n");
				CurrentLine = CleanSentence;
			}
			else
			{
				// 현재 줄에 추가
				if (!CurrentLine.IsEmpty())
				{
					CurrentLine += TEXT(" ");
				}
				CurrentLine += CleanSentence;
			}
		}
		else
		{
			// 긴 문장은 단어/글자 단위로 분리
			if (!CurrentLine.IsEmpty())
			{
				FormattedMessage += CurrentLine + TEXT("\n");
				CurrentLine.Empty();
			}

			// 공백으로 단어 분리 시도
			TArray<FString> Words;
			CleanSentence.ParseIntoArray(Words, TEXT(" "), true);

			if (Words.Num() > 1)
			{
				// 영어: 단어 단위로 처리
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
				// 한국어: 글자 단위로 강제 분리
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

	// 마지막 줄 추가
	if (!CurrentLine.IsEmpty())
	{
		FormattedMessage += CurrentLine.TrimEnd();
	}

	return FormattedMessage;
}