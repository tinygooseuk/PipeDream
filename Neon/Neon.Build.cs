// (c) 2017 TinyGoose Ltd., All Rights Reserved.

using UnrealBuildTool;

public class Neon : ModuleRules
{
	public Neon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
	}
}
