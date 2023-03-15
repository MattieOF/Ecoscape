// Copyright Epic Games, Inc. All Rights Reserved.

#include "EcoscapeGameModeBase.h"

#include "Character/EcoscapePlayerCharacter.h"
#include "Character/EcoscapePlayerController.h"

AEcoscapeGameModeBase::AEcoscapeGameModeBase()
{
	// Set default classes
	DefaultPawnClass = AEcoscapePlayerCharacter::StaticClass();
	PlayerControllerClass = AEcoscapePlayerController::StaticClass();
	DefaultPlayerName = FText::FromString("Ecoman");
}
