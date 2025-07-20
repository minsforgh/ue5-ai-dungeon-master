#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Templates/SharedPointer.h"
#include "AIManager.generated.h"

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

    // API Ű (�����Ϳ��� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FString APIKey;

private:
    void OnHttpResponse(TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bSuccess);

    FString CreateRequestBody(const FString& Message);
};