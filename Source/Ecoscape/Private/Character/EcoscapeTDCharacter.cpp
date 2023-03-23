// copyright lololol

#include "Character/EcoscapeTDCharacter.h"

#include "EcoscapeLog.h"
#include "Character/EcoscapePlayerController.h"

AEcoscapeTDCharacter::AEcoscapeTDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetWorldRotation(FRotator(-80, 0, 0));
	RootComponent = Camera;
}

void AEcoscapeTDCharacter::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	AddActorWorldOffset(WorldDirection * ScaleValue * Speed);
}

void AEcoscapeTDCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (AEcoscapePlayerController* PlayerController = Cast<AEcoscapePlayerController>(NewController))
		PlayerController->SetMouseEnabled(true);
	else
		UE_LOG(LogEcoscape, Error, TEXT("Ecoscape pawn possessed by non-ecoscape controller!"));
}
