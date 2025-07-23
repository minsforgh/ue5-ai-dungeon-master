#include "AIActionParser.h"
#include "Engine/Engine.h"

UAIActionParser::UAIActionParser()
{
    
    // 다양한 액션 형식을 위한 명령어 패턴 초기화
    CommandPatterns.Add(TEXT("^\\[(.+?)\\]"));           // [액션] 형식
    CommandPatterns.Add(TEXT("^\\*(.+?)\\*"));           // *액션* 형식  
    CommandPatterns.Add(TEXT("^Action:\\s*(.+)"));       // Action: 형식
    CommandPatterns.Add(TEXT("^You\\s+(.+?)\\.|$"));     // You [액션]. 형식
    CommandPatterns.Add(TEXT("^Player\\s+(.+?)\\.|$"));  // Player [액션]. 형식
}

void UAIActionParser::PostInitProperties()
{
    Super::PostInitProperties();
    
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        InitializeActionKeywords();
        
        UE_LOG(LogTemp, Log, TEXT("AI Action Parser initialized"));
        
        if (bDebugMode && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("AI Action Parser Ready"));
        }
    }
}

TArray<FParsedAction> UAIActionParser::ParseAIResponse(const FString& AIResponse)
{
    TArray<FParsedAction> ParsedActions;
    
    if (AIResponse.IsEmpty())
    {
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("빈 AI 응답"));
        }
        return ParsedActions;
    }
    
    // AI 응답에서 명령어들 추출
    TArray<FString> Commands = ExtractCommands(AIResponse);
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Log, TEXT("AI 응답에서 %d개의 명령어 추출"), Commands.Num());
    }
    
    // 각 명령어 파싱
    for (const FString& Command : Commands)
    {
        FParsedAction Action;
        Action.Command = CleanCommand(Command);
        Action.ActionType = ClassifyActionType(Action.Command);
        Action.Parameters = ParseParameters(Action.Command);
        Action.Target = ExtractTarget(Action.Command);
        Action.Description = AIResponse; // 컨텍스트를 위해 전체 AI 응답 저장
        
        ParsedActions.Add(Action);
        
        // 이벤트 브로드캐스트
        OnActionParsed.Broadcast(Action);
        
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Log, TEXT("파싱된 액션 - 타입: %s, 명령어: %s, 대상: %s"), 
                *UEnum::GetValueAsString(Action.ActionType), *Action.Command, *Action.Target);
        }
    }
    
    return ParsedActions;
}

TArray<FString> UAIActionParser::ExtractCommands(const FString& AIResponse)
{
    TArray<FString> Commands;
    
    // 응답을 줄별로 분할
    TArray<FString> Lines;
    AIResponse.ParseIntoArray(Lines, TEXT("\n"), true);
    
    for (const FString& Line : Lines)
    {
        FString TrimmedLine = Line.TrimStartAndEnd();
        
        if (TrimmedLine.IsEmpty())
        {
            continue;
        }
        
        // 각 패턴 확인
        bool bFoundCommand = false;
        for (const FString& Pattern : CommandPatterns)
        {
            // 간단한 패턴 매칭 (UE는 내장 정규식이 없으므로)
            if (Pattern.Contains(TEXT("^\\[")))
            {
                // [액션] 형식
                if (TrimmedLine.StartsWith(TEXT("[")) && TrimmedLine.Contains(TEXT("]")))
                {
                    int32 StartIdx = 1;
                    int32 EndIdx = TrimmedLine.Find(TEXT("]"));
                    if (EndIdx > StartIdx)
                    {
                        FString Command = TrimmedLine.Mid(StartIdx, EndIdx - StartIdx);
                        Commands.Add(Command);
                        bFoundCommand = true;
                        break;
                    }
                }
            }
            else if (Pattern.Contains(TEXT("^\\*")))
            {
                // *액션* 형식
                if (TrimmedLine.StartsWith(TEXT("*")) && TrimmedLine.EndsWith(TEXT("*")) && TrimmedLine.Len() > 2)
                {
                    FString Command = TrimmedLine.Mid(1, TrimmedLine.Len() - 2);
                    Commands.Add(Command);
                    bFoundCommand = true;
                    break;
                }
            }
            else if (Pattern.Contains(TEXT("Action:")))
            {
                // Action: 형식
                if (TrimmedLine.StartsWith(TEXT("Action:")))
                {
                    FString Command = TrimmedLine.RightChop(7).TrimStartAndEnd(); // "Action:" 제거
                    Commands.Add(Command);
                    bFoundCommand = true;
                    break;
                }
            }
            else if (Pattern.Contains(TEXT("You\\s+")))
            {
                // You [액션] 형식
                if (TrimmedLine.StartsWith(TEXT("You ")))
                {
                    FString Command = TrimmedLine.RightChop(4).TrimStartAndEnd(); // "You " 제거
                    // 끝의 마침표 제거
                    if (Command.EndsWith(TEXT(".")))
                    {
                        Command = Command.LeftChop(1);
                    }
                    Commands.Add(Command);
                    bFoundCommand = true;
                    break;
                }
            }
        }
        
        // 패턴이 매치되지 않았지만 액션처럼 보이는 경우 추가
        if (!bFoundCommand && TrimmedLine.Len() > 3)
        {
            // 줄에 액션 키워드가 포함되어 있는지 확인
            bool bContainsActionWord = false;
            for (const auto& KeywordPair : ActionKeywords)
            {
                if (ContainsActionKeyword(TrimmedLine, KeywordPair.Value))
                {
                    bContainsActionWord = true;
                    break;
                }
            }
            
            if (bContainsActionWord)
            {
                Commands.Add(TrimmedLine);
            }
        }
    }
    
    // 명령어가 발견되지 않으면 전체 응답을 명령어로 처리
    if (Commands.Num() == 0)
    {
        Commands.Add(AIResponse.TrimStartAndEnd());
    }
    
    return Commands;
}

EActionType UAIActionParser::ClassifyActionType(const FString& Command)
{
    FString LowerCommand = Command.ToLower();
    
    // Check each action type
    for (const auto& KeywordPair : ActionKeywords)
    {
        if (ContainsActionKeyword(LowerCommand, KeywordPair.Value))
        {
            return KeywordPair.Key;
        }
    }
    
    return EActionType::Unknown;
}

TArray<FString> UAIActionParser::ParseParameters(const FString& Command)
{
    TArray<FString> Parameters;
    
    // 명령어를 단어로 분할
    TArray<FString> Words;
    Command.ParseIntoArray(Words, TEXT(" "), true);
    
    // 무시할 단어들 (관사, 전치사 등)
    TArray<FString> IgnoreWords = {
        TEXT("the"), TEXT("a"), TEXT("an"), TEXT("and"), TEXT("or"), TEXT("but"),
        TEXT("is"), TEXT("are"), TEXT("was"), TEXT("were"), TEXT("be"), TEXT("been")
    };
    
    // 일반적인 매개변수 패턴 찾기
    for (int32 i = 0; i < Words.Num(); i++)
    {
        FString Word = Words[i].ToLower();
        
        // 무시할 단어들은 건너뛰기
        if (IgnoreWords.Contains(Word))
        {
            continue;
        }
        
        // 매개변수를 나타내는 전치사들
        if (Word == TEXT("to") || Word == TEXT("at") || Word == TEXT("with") || 
            Word == TEXT("on") || Word == TEXT("in") || Word == TEXT("using"))
        {
            // 다음 단어(들)이 매개변수일 수 있음 (관사 제외)
            if (i + 1 < Words.Num())
            {
                FString NextWord = Words[i + 1].ToLower();
                if (!IgnoreWords.Contains(NextWord))
                {
                    Parameters.Add(Words[i + 1]);
                }
                // "the door" 같은 경우 "door"만 추가
                else if (i + 2 < Words.Num())
                {
                    Parameters.Add(Words[i + 2]);
                }
            }
        }
        
        // 숫자를 매개변수로 처리
        if (Word.IsNumeric())
        {
            Parameters.Add(Words[i]);
        }
        
        // 따옴표 안의 단어들을 매개변수로 처리
        if (Word.StartsWith(TEXT("\"")) || Word.StartsWith(TEXT("'")))
        {
            FString QuotedParam = Words[i];
            // 닫는 따옴표까지 계속 수집
            while (i + 1 < Words.Num() && !QuotedParam.EndsWith(TEXT("\"")) && !QuotedParam.EndsWith(TEXT("'")))
            {
                i++;
                QuotedParam += TEXT(" ") + Words[i];
            }
            Parameters.Add(QuotedParam);
        }
    }
    
    return Parameters;
}

FString UAIActionParser::ExtractTarget(const FString& Command)
{
    FString LowerCommand = Command.ToLower();
    TArray<FString> Words;
    Command.ParseIntoArray(Words, TEXT(" "), true);
    
    // Look for target indicators
    for (int32 i = 0; i < Words.Num(); i++)
    {
        FString Word = Words[i].ToLower();
        
        if (Word == TEXT("the") || Word == TEXT("a") || Word == TEXT("an"))
        {
            if (i + 1 < Words.Num())
            {
                FString PotentialTarget = Words[i + 1];
                
                // Common target types
                if (PotentialTarget.ToLower().Contains(TEXT("door")) ||
                    PotentialTarget.ToLower().Contains(TEXT("chest")) ||
                    PotentialTarget.ToLower().Contains(TEXT("enemy")) ||
                    PotentialTarget.ToLower().Contains(TEXT("monster")) ||
                    PotentialTarget.ToLower().Contains(TEXT("item")) ||
                    PotentialTarget.ToLower().Contains(TEXT("npc")) ||
                    PotentialTarget.ToLower().Contains(TEXT("lever")) ||
                    PotentialTarget.ToLower().Contains(TEXT("button")))
                {
                    return PotentialTarget;
                }
            }
        }
        
        // Direct target references
        if (Word == TEXT("attack") || Word == TEXT("hit") || Word == TEXT("strike"))
        {
            if (i + 1 < Words.Num())
            {
                return Words[i + 1];
            }
        }
    }
    
    // If no specific target found, return last word (might be target)
    if (Words.Num() > 1)
    {
        FString LastWord = Words.Last().ToLower();
        if (!LastWord.Contains(TEXT("ly")) && !LastWord.Contains(TEXT("ing"))) // Avoid adverbs/gerunds
        {
            return Words.Last();
        }
    }
    
    return TEXT("");
}

FString UAIActionParser::CleanCommand(const FString& RawCommand)
{
    FString Cleaned = RawCommand.TrimStartAndEnd();
    
    // 일반적인 접두사 제거
    if (Cleaned.StartsWith(TEXT("Player ")))
    {
        Cleaned = Cleaned.RightChop(7);
    }
    else if (Cleaned.StartsWith(TEXT("You ")))
    {
        Cleaned = Cleaned.RightChop(4);
    }
    
    // 끝의 구두점 제거
    while (Cleaned.EndsWith(TEXT(".")) || Cleaned.EndsWith(TEXT("!")) || Cleaned.EndsWith(TEXT("?")))
    {
        Cleaned = Cleaned.LeftChop(1);
    }
    
    return Cleaned.TrimStartAndEnd();
}

bool UAIActionParser::ContainsActionKeyword(const FString& Command, const TArray<FString>& Keywords)
{
    FString LowerCommand = Command.ToLower();
    
    for (const FString& Keyword : Keywords)
    {
        if (LowerCommand.Contains(Keyword.ToLower()))
        {
            return true;
        }
    }
    
    return false;
}

TArray<FString> UAIActionParser::GetActionKeywords(EActionType ActionType)
{
    if (ActionKeywords.Contains(ActionType))
    {
        return ActionKeywords[ActionType];
    }
    
    return TArray<FString>();
}

void UAIActionParser::InitializeActionKeywords()
{
    ActionKeywords.Empty();
    
    // 이동 액션들
    ActionKeywords.Add(EActionType::Move, {
        TEXT("move"), TEXT("walk"), TEXT("run"), TEXT("go"), TEXT("travel"), 
        TEXT("step"), TEXT("proceed"), TEXT("advance"), TEXT("retreat"), 
        TEXT("approach"), TEXT("leave"), TEXT("exit"), TEXT("enter")
    });
    
    // 공격 액션들
    ActionKeywords.Add(EActionType::Attack, {
        TEXT("attack"), TEXT("hit"), TEXT("strike"), TEXT("fight"), TEXT("battle"),
        TEXT("slash"), TEXT("stab"), TEXT("shoot"), TEXT("fire"), TEXT("swing"),
        TEXT("punch"), TEXT("kick"), TEXT("charge"), TEXT("assault")
    });
    
    // 상호작용 액션들
    ActionKeywords.Add(EActionType::Interact, {
        TEXT("open"), TEXT("close"), TEXT("push"), TEXT("pull"), TEXT("press"),
        TEXT("touch"), TEXT("grab"), TEXT("take"), TEXT("pick"), TEXT("lift"),
        TEXT("activate"), TEXT("use"), TEXT("operate"), TEXT("interact")
    });
    
    // 관찰 액션들
    ActionKeywords.Add(EActionType::Look, {
        TEXT("look"), TEXT("examine"), TEXT("inspect"), TEXT("observe"), TEXT("check"),
        TEXT("search"), TEXT("scan"), TEXT("peek"), TEXT("watch"), TEXT("study")
    });
    
    // 아이템 사용 액션들
    ActionKeywords.Add(EActionType::UseItem, {
        TEXT("use"), TEXT("drink"), TEXT("eat"), TEXT("consume"), TEXT("apply"),
        TEXT("wield"), TEXT("equip"), TEXT("wear"), TEXT("hold"), TEXT("drop")
    });
    
    // 인벤토리 액션들
    ActionKeywords.Add(EActionType::Inventory, {
        TEXT("inventory"), TEXT("items"), TEXT("bag"), TEXT("backpack"), TEXT("check items"),
        TEXT("list items"), TEXT("show items")
    });
    
    // 대화 액션들
    ActionKeywords.Add(EActionType::Talk, {
        TEXT("talk"), TEXT("speak"), TEXT("say"), TEXT("tell"), TEXT("ask"),
        TEXT("chat"), TEXT("converse"), TEXT("discuss"), TEXT("question")
    });
    
    // 마법 시전 액션들
    ActionKeywords.Add(EActionType::Cast, {
        TEXT("cast"), TEXT("spell"), TEXT("magic"), TEXT("enchant"), TEXT("summon"),
        TEXT("invoke"), TEXT("conjure"), TEXT("channel"), TEXT("hex"), TEXT("curse")
    });
    
    // 대기 액션들
    ActionKeywords.Add(EActionType::Wait, {
        TEXT("wait"), TEXT("rest"), TEXT("pause"), TEXT("delay"), TEXT("stay"),
        TEXT("remain"), TEXT("hold"), TEXT("stop"), TEXT("idle")
    });
}