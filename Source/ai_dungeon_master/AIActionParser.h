#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "AIActionParser.generated.h"

// 액션 타입 열거형
UENUM(BlueprintType)
enum class EActionType : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),     // 알 수 없는 액션
    Move        UMETA(DisplayName = "Move"),        // 이동 액션
    Attack      UMETA(DisplayName = "Attack"),      // 공격 액션
    Interact    UMETA(DisplayName = "Interact"),    // 상호작용 액션
    Look        UMETA(DisplayName = "Look"),        // 관찰 액션
    UseItem     UMETA(DisplayName = "Use Item"),    // 아이템 사용 액션
    Inventory   UMETA(DisplayName = "Inventory"),   // 인벤토리 액션
    Talk        UMETA(DisplayName = "Talk"),        // 대화 액션
    Cast        UMETA(DisplayName = "Cast Spell"),  // 마법 시전 액션
    Wait        UMETA(DisplayName = "Wait")         // 대기 액션
};

// 파싱된 액션 구조체
USTRUCT(BlueprintType)
struct FParsedAction
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EActionType ActionType = EActionType::Unknown;  // 액션 타입

    UPROPERTY(BlueprintReadOnly)
    FString Command;                                // 원본 명령어

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Parameters;                     // 명령어 매개변수들

    UPROPERTY(BlueprintReadOnly)
    FString Target;                                 // 액션 대상

    UPROPERTY(BlueprintReadOnly)
    FString Description;                            // 액션 설명

    FParsedAction()
    {
        ActionType = EActionType::Unknown;
        Command = TEXT("");
        Parameters.Empty();
        Target = TEXT("");
        Description = TEXT("");
    }
};

// 액션 파싱 완료 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionParsed, const FParsedAction&, ParsedAction);

UCLASS()
class AI_DUNGEON_MASTER_API UAIActionParser : public UObject
{
    GENERATED_BODY()

public:
    UAIActionParser();

protected:
    virtual void PostInitProperties() override;

public:
    // 메인 파싱 함수 - AI 응답을 분석하여 액션들로 변환
    UFUNCTION(BlueprintCallable, Category = "AI Action Parser")
    TArray<FParsedAction> ParseAIResponse(const FString& AIResponse);

    // AI 응답에서 개별 명령어들 추출
    UFUNCTION(BlueprintCallable, Category = "AI Action Parser")
    TArray<FString> ExtractCommands(const FString& AIResponse);

    // 명령어로부터 액션 타입 분류
    UFUNCTION(BlueprintCallable, Category = "AI Action Parser")
    EActionType ClassifyActionType(const FString& Command);

    // 명령어에서 매개변수 파싱
    UFUNCTION(BlueprintCallable, Category = "AI Action Parser")
    TArray<FString> ParseParameters(const FString& Command);

    // 명령어에서 대상 추출
    UFUNCTION(BlueprintCallable, Category = "AI Action Parser")
    FString ExtractTarget(const FString& Command);

    // 액션 파싱 완료 이벤트
    UPROPERTY(BlueprintAssignable, Category = "AI Action Parser")
    FOnActionParsed OnActionParsed;

private:
    // 헬퍼 함수들
    FString CleanCommand(const FString& RawCommand);                                            // 명령어 정리 함수
    bool ContainsActionKeyword(const FString& Command, const TArray<FString>& Keywords);       // 액션 키워드 포함 확인
    TArray<FString> GetActionKeywords(EActionType ActionType);                                 // 액션 타입별 키워드 가져오기
    
    // 액션 키워드 매핑 초기화
    void InitializeActionKeywords();
    TMap<EActionType, TArray<FString>> ActionKeywords;                                         // 액션 타입별 키워드 맵

    // 명령어 추출을 위한 정규식 패턴들
    TArray<FString> CommandPatterns;
    
    // 디버그 모드
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugMode = true;
};