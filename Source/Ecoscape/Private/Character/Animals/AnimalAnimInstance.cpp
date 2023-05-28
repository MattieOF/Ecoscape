// copyright lololol

#include "Character/Animals/AnimalAnimInstance.h"

#include "Ecoscape.h"
#include "EcoscapeStatics.h"
#include "Character/Animals/BaseAnimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#if WITH_EDITOR
#include "MessageLogModule.h"
#endif

void UAnimalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	// Get animal
	const ABaseAnimal* Animal = Cast<ABaseAnimal>(TryGetPawnOwner());
	if (!Animal)
	{
		if (UEcoscapeStatics::InActualGame())
		{
			ECO_LOG_ERROR("In UAnimalAnimInstance, pawn owner is not a BaseAnimal!");
		}
		return;
	}

	// Update move speed
	const UCharacterMovementComponent* CharacterMovement = Animal->GetCharacterMovement();
	float TargetMoveSpeed = 0;
	if (CharacterMovement->MaxWalkSpeed > 0)
		TargetMoveSpeed = CharacterMovement->Velocity.Length() / CharacterMovement->MaxWalkSpeed;
	MoveSpeed = UKismetMathLibrary::FInterpTo_Constant(MoveSpeed, TargetMoveSpeed, DeltaSeconds, 1.5f);

	// Update other state variables
	bIsEating   = Animal->bIsEating;
	bIsDrinking = Animal->bIsDrinking;
	bIsSleeping = Animal->bIsSleeping;
	bIsDead     = Animal->Health <= 0;
}
