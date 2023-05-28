// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Ecoscape : ModuleRules
{
	public Ecoscape(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "PhysicsCore", "UMG", "AIModule", "GameplayTasks" });
		PrivateDependencyModuleNames.AddRange(new string[] { "ProceduralMeshComponent", "GeometryFramework", "GeometryScriptingCore", "NavigationSystem" });
		
		if (Target.bBuildEditor) 
		{
			PublicDependencyModuleNames.Add("UnrealEd");
			PrivateDependencyModuleNames.Add("MessageLog");
		}
	}
}
