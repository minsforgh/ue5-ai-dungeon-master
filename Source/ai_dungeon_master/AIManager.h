#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "AIActionParser.h"
#include "AIManager.generated.h"

class IHttpRequest;
class IHttpResponse;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIResponse, bool, bSuccess, const FString&, Response);

UCLASS()
class AI_DUNGEON_MASTER_API AAIManager : public AActor
{
    GENERATED_BODY()

public:
    AAIManager();

protected:
    virtual void BeginPlay() override;

public:
    // AI에게 메시지 전송
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SendMessage(const FString& Message);

    // AI 응답 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "AI")
    FOnAIResponse OnAIResponse;

    // API 키 가져오기 (테스트 목적)
    UFUNCTION(BlueprintCallable, Category = "OpenAI")
    FString GetAPIKey();
    
    // 액션 파서 테스트 함수
    UFUNCTION(BlueprintCallable, Category = "AI Action Parser")
    void TestActionParser(const FString& TestInput);
    
    // 콘솔 명령어로 테스트
    UFUNCTION(Exec)
    void TestParser(const FString& Input);

private:
    // HTTP 응답 처리
    void OnHttpResponse(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bSuccess);

    // JSON 요청 본문 생성
    FString CreateRequestBody(const FString& Message);

    // 안전한 파일 읽기 (단계별)
    FString LoadAPIKeyFromFile();
    bool IsEngineReady();
    bool DoesFileExistSafely(const FString& FilePath);
    FString ReadFileContentSafely(const FString& FilePath);

    // 캐시된 API 키
    FString APIKey;

    // API 키 로드 상태
    bool bAPIKeyLoaded = false;
    
    // 액션 파서 레퍼런스
    UPROPERTY()
    class UAIActionParser* ActionParser;
};