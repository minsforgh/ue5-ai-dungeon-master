// Copyright Epic Games, Inc. All Rights Reserved.


#include "ai_dungeon_masterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void Aai_dungeon_masterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Contexts
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

void Aai_dungeon_masterPlayerController::TestParser(const FString& Input)
{
	AAIManager* AIManager = GetAIManager();
	if (AIManager)
	{
		AIManager->TestActionParser(Input);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AI Manager를 찾을 수 없습니다"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("AI Manager 없음"));
		}
	}
}

AAIManager* Aai_dungeon_masterPlayerController::GetAIManager()
{
	if (UWorld* World = GetWorld())
	{
		// 월드에서 AI Manager 찾기
		for (TActorIterator<AAIManager> ActorItr(World); ActorItr; ++ActorItr)
		{
			return *ActorItr;
		}
	}
	return nullptr;
}
