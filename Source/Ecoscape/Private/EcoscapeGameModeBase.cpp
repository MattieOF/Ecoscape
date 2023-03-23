// Copyright Epic Games, Inc. All Rights Reserved.

#include "EcoscapeGameModeBase.h"

#include "Character/EcoscapeFPPlayerCharacter.h"
#include "Character/EcoscapePlayerController.h"

AEcoscapeGameModeBase::AEcoscapeGameModeBase()
{
	// Set default classes
	DefaultPawnClass = AEcoscapeFPPlayerCharacter::StaticClass();
	PlayerControllerClass = AEcoscapePlayerController::StaticClass();
	DefaultPlayerName = FText::FromString("Ecoman");
}
