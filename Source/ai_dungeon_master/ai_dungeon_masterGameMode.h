// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AIManager.h"
#include "ai_dungeon_masterGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class Aai_dungeon_masterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	Aai_dungeon_masterGameMode();

protected:
	virtual void BeginPlay() override;

public:
	// 액션 파서 테스트용 콘솔 명령어 (GameMode에서)
	UFUNCTION(Exec)
	void TestActionParser(const FString& Input);

private:
	// AI Manager 레퍼런스
	UPROPERTY()
	AAIManager* AIManager;
};



