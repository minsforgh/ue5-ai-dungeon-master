#include "AIManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/Engine.h"

AAIManager::AAIManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AAIManager::BeginPlay()
{
    Super::BeginPlay();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("AI Manager Ready!"));
    }
    UE_LOG(LogTemp, Log, TEXT("AI Manager ready"));
}

void AAIManager::SendMessage(const FString& Message)
{
    if (APIKey.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("API Key not set"));
        OnAIResponse.Broadcast(false, TEXT("API Key missing"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("API Key missing!"));
        }
        return;
    }

    if (Message.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Empty message"));
        return;
    }

    // HTTP 요청 생성
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &AAIManager::OnHttpResponse);

    Request->SetURL(TEXT("https://api.openai.com/v1/chat/completions"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *APIKey));

    // JSON 요청 생성
    Request->SetContentAsString(CreateRequestBody(Message));
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("Sending: %s"), *Message);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Sending to AI..."));
    }
}

void AAIManager::OnHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP request failed"));
        OnAIResponse.Broadcast(false, TEXT("Network error"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Network error"));
        }
        return;
    }

    int32 ResponseCode = Response->GetResponseCode();
    if (ResponseCode != 200)
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP error code: %d"), ResponseCode);
        OnAIResponse.Broadcast(false, FString::Printf(TEXT("HTTP Error: %d"), ResponseCode));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                FString::Printf(TEXT("HTTP Error: %d"), ResponseCode));
        }
        return;
    }

    // JSON 파싱
    FString ResponseString = Response->GetContentAsString();
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        const TArray<TSharedPtr<FJsonValue>>* Choices;
        if (JsonObject->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
        {
            TSharedPtr<FJsonObject> Message = (*Choices)[0]->AsObject()->GetObjectField(TEXT("message"));
            if (Message.IsValid())
            {
                FString Content = Message->GetStringField(TEXT("content"));

                OnAIResponse.Broadcast(true, Content);
                UE_LOG(LogTemp, Log, TEXT("AI Response: %s"), *Content);

                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
                        FString::Printf(TEXT("AI: %s"), *Content));
                }
                return;
            }
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Failed to parse AI response"));
    OnAIResponse.Broadcast(false, TEXT("Parse error"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Parse error"));
    }
}

FString AAIManager::CreateRequestBody(const FString& Message)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("model"), TEXT("gpt-3.5-turbo"));
    JsonObject->SetNumberField(TEXT("max_tokens"), 100);
    JsonObject->SetNumberField(TEXT("temperature"), 0.7);

    TArray<TSharedPtr<FJsonValue>> Messages;

    // 시스템 메시지
    TSharedPtr<FJsonObject> SystemMsg = MakeShareable(new FJsonObject);
    SystemMsg->SetStringField(TEXT("role"), TEXT("system"));
    SystemMsg->SetStringField(TEXT("content"), TEXT("You are a dungeon master for a text adventure game. Keep responses concise and engaging (under 50 words)."));
    Messages.Add(MakeShareable(new FJsonValueObject(SystemMsg)));

    // 사용자 메시지
    TSharedPtr<FJsonObject> UserMsg = MakeShareable(new FJsonObject);
    UserMsg->SetStringField(TEXT("role"), TEXT("user"));
    UserMsg->SetStringField(TEXT("content"), Message);
    Messages.Add(MakeShareable(new FJsonValueObject(UserMsg)));

    JsonObject->SetArrayField(TEXT("messages"), Messages);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    return OutputString;
}