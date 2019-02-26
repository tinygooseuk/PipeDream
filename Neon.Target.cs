// (c) 2017 TinyGoose Ltd., All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonTarget : TargetRules
{
	public NeonTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("Neon");
	}
}
