// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;

public class ai_dungeon_master : ModuleRules
{
    public ai_dungeon_master(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 기본 필수 모듈들
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",	// 입력 시스템
			"UMG",				// UI 시스템
			"Slate",			// UI 프레임워크
			"SlateCore"			// UI 프레임워크 코어
		});

        // AI 던전 마스터 프로젝트용 모듈들
        PrivateDependencyModuleNames.AddRange(new string[] {
            "HTTP",				// OpenAI API 통신
			"Json",				// JSON 파싱
			"JsonUtilities",	// JSON 유틸리티
			"ApplicationCore",	// 애플리케이션 코어
			"RHI",				// 렌더링 (머티리얼 조작용)
			"RenderCore"		// 렌더링 코어
		});

        // 조건부 에디터 모듈들
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "EditorStyle",
                "EditorWidgets",
                "UnrealEd"
            });
        }

        // HTTP 모듈 활성화
        PublicDefinitions.Add("WITH_HTTP=1");

        // JSON 지원 활성화  
        PublicDefinitions.Add("WITH_JSON=1");
    }
}