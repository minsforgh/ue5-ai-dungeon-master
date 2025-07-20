// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;

public class ai_dungeon_master : ModuleRules
{
    public ai_dungeon_master(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // �⺻ �ʼ� ����
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",	// �Է� �ý���
			"UMG",				// UI �ý���
			"Slate",			// UI �����ӿ�ũ
			"SlateCore"			// UI �����ӿ�ũ �ھ�
		});

        // AI ���� ������ ������Ʈ�� ����
        PrivateDependencyModuleNames.AddRange(new string[] {
            "HTTP",				// OpenAI API ���
			"Json",				// JSON �Ľ�
			"JsonUtilities",	// JSON ��ƿ��Ƽ
			"ApplicationCore",	// ���ø����̼� �ھ�
			"RHI",				// ������ (��Ƽ���� ���ۿ�)
			"RenderCore"		// ������ �ھ�
		});

        // ���Ǻ� ������ ����
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "EditorStyle",
                "EditorWidgets",
                "UnrealEd"
            });
        }

        // HTTP ��� Ȱ��ȭ
        PublicDefinitions.Add("WITH_HTTP=1");

        // JSON ���� Ȱ��ȭ  
        PublicDefinitions.Add("WITH_JSON=1");
    }
}