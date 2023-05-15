// Copyright Epic Games, Inc. All Rights Reserved.

#include "EcoscapeGameModeBase.h"

#include "EcoscapeStatics.h"
#include "Character/EcoscapeFPPlayerCharacter.h"
#include "Character/EcoscapePlayerController.h"
#include "Character/Animals/BaseAnimal.h"
#include "World/EcoscapeTerrain.h"

AEcoscapeGameModeBase::AEcoscapeGameModeBase()
{
	// Set default classes
	DefaultPawnClass = AEcoscapeFPPlayerCharacter::StaticClass();
	PlayerControllerClass = AEcoscapePlayerController::StaticClass();
	DefaultPlayerName = FText::FromString("Ecoman");
}

void AEcoscapeGameModeBase::SpawnAnimalAtCursor(FString Animal)
{
	const AEcoscapePlayerController* Controller = AEcoscapePlayerController::GetEcoscapePlayerController(GetWorld());
	FHitResult Hit;
	if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Controller, ECC_ITEM_PLACEABLE_ON, false, Hit, TArray<AActor*>()))
	{
		UAnimalData* AnimalData = UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld())->AnimalTypes[Animal];
		ABaseAnimal::SpawnAnimal(GetWorld(), AnimalData, Cast<AEcoscapeTerrain>(Hit.GetActor()), Hit.ImpactPoint + FVector(0, 0, 100));
	}
}
