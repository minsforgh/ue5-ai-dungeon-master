// AIManager.h - ������ ���� �б� ���� ����
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
    // AI���� �޽��� ����
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SendMessage(const FString& Message);

    // AI ���� ��������Ʈ
    UPROPERTY(BlueprintAssignable, Category = "AI")
    FOnAIResponse OnAIResponse;

    // API Ű �������� (������ ����)
    UFUNCTION(BlueprintCallable, Category = "OpenAI")
    FString GetAPIKey();

private:
    // HTTP ���� ó��
    void OnHttpResponse(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bSuccess);

    // JSON ��û ���� ����
    FString CreateRequestBody(const FString& Message);

    // ������ ���� �б� (�ܰ躰)
    FString LoadAPIKeyFromFile();
    bool IsEngineReady();
    bool DoesFileExistSafely(const FString& FilePath);
    FString ReadFileContentSafely(const FString& FilePath);

    // ĳ�õ� API Ű
    FString APIKey;

    // API Ű �ε� ����
    bool bAPIKeyLoaded = false;
};