#include "AIManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

AAIManager::AAIManager()
{
    PrimaryActorTick.bCanEverTick = false;
    ActionParser = nullptr;
}

void AAIManager::BeginPlay()
{
    Super::BeginPlay();

    // 액션 파서 생성 (UObject로 안전하게)
    if (!ActionParser)
    {
        ActionParser = NewObject<UAIActionParser>(this);
        if (ActionParser)
        {
            UE_LOG(LogTemp, Log, TEXT("액션 파서 생성 완료"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("액션 파서 생성 실패"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AI Manager initialized - ready for use"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("AI Manager Ready"));
    }

    // 자동 테스트 실행 (디버그용)
    FTimerHandle TestTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TestTimerHandle, [this]()
    {
        if (ActionParser)
        {
            UE_LOG(LogTemp, Warning, TEXT("=== 자동 테스트 시작 ==="));
            TestActionParser(TEXT("move to the door"));
            TestActionParser(TEXT("[attack orc]"));
            TestActionParser(TEXT("*look around*"));
        }
    }, 2.0f, false);
}

void AAIManager::SendMessage(const FString& Message)
{
    // API 키 확인
    FString CurrentAPIKey = GetAPIKey();

    if (CurrentAPIKey.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("API Key not available"));
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
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *CurrentAPIKey));

    // JSON 요청 생성
    Request->SetContentAsString(CreateRequestBody(Message));
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("Sending: %s"), *Message);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Sending to AI..."));
    }
}

void AAIManager::OnHttpResponse(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
    TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
    bool bSuccess)
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

FString AAIManager::GetAPIKey()
{
    // 이미 로드된 경우 캐시된 키 반환
    if (bAPIKeyLoaded && !APIKey.IsEmpty())
    {
        return APIKey;
    }

    // 재 시도 방지하도록 플래그 설정
    if (bAPIKeyLoaded)
    {
        return TEXT("");
    }

    // 안전한 파일 읽기 시도
    FString LoadedKey = LoadAPIKeyFromFile();

    if (!LoadedKey.IsEmpty())
    {
        APIKey = LoadedKey;
        UE_LOG(LogTemp, Log, TEXT("API key loaded successfully from file"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not load API key from file"));
    }

    bAPIKeyLoaded = true;
    return APIKey;
}

FString AAIManager::LoadAPIKeyFromFile()
{
    UE_LOG(LogTemp, Log, TEXT("=== Starting safe API key loading ==="));

    // 1단계: 엔진 준비 상태 확인
    if (!IsEngineReady())
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine not ready for file operations"));
        return TEXT("");
    }

    // 2단계: 파일 경로 생성
    FString FilePath;
    try
    {
        FilePath = FPaths::ProjectDir() + TEXT("api_key.txt");
        UE_LOG(LogTemp, Log, TEXT("Target file path: %s"), *FilePath);
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create file path"));
        return TEXT("");
    }

    // 3단계: 파일 존재 확인
    if (!DoesFileExistSafely(FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("API key file does not exist: %s"), *FilePath);
        UE_LOG(LogTemp, Warning, TEXT("Please create api_key.txt in project root with your OpenAI API key"));
        return TEXT("");
    }

    // 4�ܰ�: ���� ���� �б�
    FString FileContent = ReadFileContentSafely(FilePath);

    if (FileContent.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read file or file is empty"));
        return TEXT("");
    }

    // 5단계: API 키 유효성 검사
    FString CleanedKey = FileContent.TrimStartAndEnd();

    if (CleanedKey.StartsWith(TEXT("sk-")) && CleanedKey.Len() > 20)
    {
        UE_LOG(LogTemp, Log, TEXT("Valid API key loaded (length: %d)"), CleanedKey.Len());
        return CleanedKey;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid API key format. Must start with 'sk-' and be longer than 20 characters"));
        return TEXT("");
    }
}

bool AAIManager::IsEngineReady()
{
    // 기본적인 엔진 상태 확인
    if (!GEngine || !IsValid(this))
    {
        return false;
    }

    // 월드 상태 확인
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    return true;
}

bool AAIManager::DoesFileExistSafely(const FString& FilePath)
{
    try
    {
        // ���� ������ ���: FPaths ���
        return FPaths::FileExists(FilePath);
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception during file existence check"));
        return false;
    }
}

FString AAIManager::ReadFileContentSafely(const FString& FilePath)
{
    try
    {
        // ��� 1: ����Ʈ �迭�� �б� (�� ����)
        TArray<uint8> FileData;
        if (FFileHelper::LoadFileToArray(FileData, *FilePath))
        {
            // null terminator �߰�
            FileData.Add(0);

            // UTF8���� FString���� ��ȯ
            FString Result = FString(UTF8_TO_TCHAR(FileData.GetData()));
            UE_LOG(LogTemp, Log, TEXT("File read successfully using byte array method"));
            return Result;
        }

        // ��� 2: ���� ���ڿ� �б� (���)
        FString DirectResult;
        if (FFileHelper::LoadFileToString(DirectResult, *FilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("File read successfully using direct string method"));
            return DirectResult;
        }

        UE_LOG(LogTemp, Error, TEXT("Both file reading methods failed"));
        return TEXT("");
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception during file reading"));
        return TEXT("");
    }
}

void AAIManager::TestActionParser(const FString& TestInput)
{
    if (!ActionParser)
    {
        UE_LOG(LogTemp, Error, TEXT("액션 파서가 초기화되지 않았습니다"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("액션 파서 없음"));
        }
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== 액션 파서 테스트 시작 ==="));
    UE_LOG(LogTemp, Log, TEXT("입력: %s"), *TestInput);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("테스트 입력: %s"), *TestInput));
    }
    
    // AI 응답 파싱
    TArray<FParsedAction> ParsedActions = ActionParser->ParseAIResponse(TestInput);
    
    UE_LOG(LogTemp, Log, TEXT("파싱된 액션 수: %d"), ParsedActions.Num());
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            FString::Printf(TEXT("파싱된 액션 수: %d"), ParsedActions.Num()));
    }
    
    // 각 액션 정보 출력
    for (int32 i = 0; i < ParsedActions.Num(); i++)
    {
        const FParsedAction& Action = ParsedActions[i];
        
        FString ActionTypeStr = UEnum::GetValueAsString(Action.ActionType);
        
        UE_LOG(LogTemp, Log, TEXT("액션 %d:"), i + 1);
        UE_LOG(LogTemp, Log, TEXT("  타입: %s"), *ActionTypeStr);
        UE_LOG(LogTemp, Log, TEXT("  명령어: %s"), *Action.Command);
        UE_LOG(LogTemp, Log, TEXT("  대상: %s"), *Action.Target);
        UE_LOG(LogTemp, Log, TEXT("  매개변수 수: %d"), Action.Parameters.Num());
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan,
                FString::Printf(TEXT("액션 %d: %s - %s"), i + 1, *ActionTypeStr, *Action.Command));
        }
        
        // 매개변수들 출력
        for (int32 j = 0; j < Action.Parameters.Num(); j++)
        {
            UE_LOG(LogTemp, Log, TEXT("    매개변수 %d: %s"), j + 1, *Action.Parameters[j]);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== 액션 파서 테스트 완료 ==="));
}

void AAIManager::TestParser(const FString& Input)
{
    TestActionParser(Input);
}