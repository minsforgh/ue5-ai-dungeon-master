// AIManager.h - 안전한 파일 읽기 최종 버전
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
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

    // API 키 가져오기 (안전한 버전)
    UFUNCTION(BlueprintCallable, Category = "OpenAI")
    FString GetAPIKey();

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
};