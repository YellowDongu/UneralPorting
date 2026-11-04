using UnrealBuildTool;

public class CustomEditor : ModuleRules
{
    public CustomEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "UnrealEd",
                "EditorFramework",
                "InputCore",
                "LevelEditor",
                "AC7"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "ToolMenus",
                "ContentBrowser",
                "AssetRegistry",
                "LevelEditor"
            }
        );
    }
}