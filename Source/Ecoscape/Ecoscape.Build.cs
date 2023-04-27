// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Ecoscape : ModuleRules
{
	public Ecoscape(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "PhysicsCore", "UMG" });
		PrivateDependencyModuleNames.AddRange(new string[] { "ProceduralMeshComponent", "GeometryFramework", "GeometryScriptingCore" });
	}
}
