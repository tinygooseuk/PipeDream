// (c) 2017 TinyGoose Ltd., All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonEditorTarget : TargetRules
{
	public NeonEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("Neon");
	}
}
