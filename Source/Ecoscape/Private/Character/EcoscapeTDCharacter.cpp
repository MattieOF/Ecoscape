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

void AEcoscapeTDCharacter::AddScrollInput(float Value)
{
	TargetHeight = FMath::Clamp(TargetHeight + -Value * ZoomSensitivity, HeightBounds.X, HeightBounds.Y);
}

void AEcoscapeTDCharacter::BeginPlay()
{
	Super::BeginPlay();
	TargetHeight = GetActorLocation().Z;
}

void AEcoscapeTDCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector Location = GetActorLocation();
	if (Location.Z != TargetHeight)
	{
		const float Diff = Location.Z - TargetHeight;
		if (FMath::Abs(Diff) < 1)
			Location.Z = TargetHeight;
		else
			Location.Z += -Diff * 0.15f;
		SetActorLocation(Location);	
	}
}

void AEcoscapeTDCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (AEcoscapePlayerController* PlayerController = Cast<AEcoscapePlayerController>(NewController))
		PlayerController->SetMouseEnabled(true);
	else
		UE_LOG(LogEcoscape, Error, TEXT("Ecoscape pawn possessed by non-ecoscape controller!"));
}
