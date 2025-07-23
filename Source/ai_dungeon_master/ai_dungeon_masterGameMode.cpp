// Copyright Epic Games, Inc. All Rights Reserved.

#include "ai_dungeon_masterGameMode.h"

Aai_dungeon_masterGameMode::Aai_dungeon_masterGameMode()
{
	AIManager = nullptr;
}

void Aai_dungeon_masterGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// AI Manager 생성
	if (!AIManager)
	{
		AIManager = GetWorld()->SpawnActor<AAIManager>();
		if (AIManager)
		{
			UE_LOG(LogTemp, Log, TEXT("GameMode에서 AI Manager 생성 완료"));
		}
	}
}

void Aai_dungeon_masterGameMode::TestActionParser(const FString& Input)
{
	if (AIManager)
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode 테스트 명령어 실행: %s"), *Input);
		AIManager->TestActionParser(Input);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AI Manager가 초기화되지 않았습니다"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("AI Manager 없음 (GameMode)"));
		}
	}
}
