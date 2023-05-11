// copyright lololol

#include "Character/Animals/BaseAnimal.h"

#include "Ecoscape.h"
#include "EcoscapeStatics.h"
#include "MessageLogModule.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

ABaseAnimal::ABaseAnimal()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->RotationRate.Yaw = 180;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// Enable avoidance
	GetCharacterMovement()->bUseRVOAvoidance = true;

	GetMesh()->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	GetMesh()->SetCollisionResponseToChannel(ECC_BLOCKS_ITEM_PLACEMENT, ECR_Block);

	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_Yes;
	GetMesh()->CanCharacterStepUpOn = ECB_Yes;
}

void ABaseAnimal::BeginPlay()
{
	Super::BeginPlay();
	
	if (!AnimalData)
	{
		ECO_LOG_ERROR(FString::Printf(TEXT("Null AnimalData in %s"), *GetName()));
		Destroy();
		return;
	}

	SetAnimalData(AnimalData);
}

void ABaseAnimal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Do hunger
	if (Hunger <= 0)
		Health -= 1 * DeltaSeconds; // Panda will die in ~60 seconds from no food
	else
		Hunger = FMath::Max(0, Hunger -= DeltaSeconds * AnimalData->HungerRate);

	if (Health <= 0)
	{
		OnDeath.Broadcast();
		Destroy();
	}

	DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 100),
	                FString::Printf(TEXT("HP: %f, Hunger: %f"), Health, Hunger), nullptr, FColor::White, DeltaSeconds);

	// Find target rotation
	FRotator CurrentRotation = GetMesh()->GetComponentRotation();
	double CurrentYaw = CurrentRotation.Yaw;
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), GetActorLocation() + FVector(0, 0, -350),
	                                         ECC_ITEM_PLACEABLE_ON))
	{
		GetMesh()->SetWorldRotation(FRotator(0, CurrentYaw, 0));
		
		// Set mesh rotation to match terrain
		auto Rotation = UKismetMathLibrary::Conv_VectorToRotator(Hit.ImpactNormal);
		FVector NormalVector = UKismetMathLibrary::VLerp(FVector::UpVector, Hit.ImpactNormal, 0.4f);
		FVector RotationAxis = FVector::CrossProduct(FVector::UpVector, NormalVector);
		RotationAxis.Normalize();
		float DotProduct = FVector::DotProduct(FVector::UpVector, NormalVector);
		float RotationAngle = acosf(DotProduct);
		FQuat Quat = FQuat(RotationAxis, RotationAngle);
		FQuat RootQuat = GetMesh()->GetComponentQuat();
		FQuat NewQuat = Quat * RootQuat;
		Rotation = NewQuat.Rotator();
		Rotation.Yaw = CurrentYaw;
		TargetRotation = Rotation;

		GetMesh()->SetWorldRotation(CurrentRotation);
	}
	else
		TargetRotation = FRotator(0, CurrentYaw, 0);

	// Interpolate towards target rotation
	GetMesh()->SetWorldRotation(UKismetMathLibrary::RInterpTo_Constant(CurrentRotation, TargetRotation, DeltaSeconds, 15));
}

#if WITH_EDITOR
void ABaseAnimal::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetNameCPP() == "AnimalData")
		SetAnimalData(AnimalData, false);
}
#endif

void ABaseAnimal::SetAnimalData(UAnimalData* Data, bool bRecreateAI)
{
	AnimalData = Data;
	
	if (Data)
	{
		// Setup basic data
		GivenName = Data->SpeciesName.ToString();
		Health = Data->BaseHealth;
		Hunger = 1;
		GetCharacterMovement()->MaxWalkSpeed = Data->MoveSpeed;

		// Setup mesh
		GetMesh()->SetSkeletalMesh(Data->Mesh);
		GetMesh()->SetAnimInstanceClass(Data->AnimationClass);
		GetMesh()->SetRelativeLocation(Data->MeshOffset);
		GetMesh()->SetWorldScale3D(Data->MeshScale);

		// Setup capsule
		GetCapsuleComponent()->SetCapsuleHalfHeight(Data->ColliderHalfHeight);
		GetCapsuleComponent()->SetCapsuleRadius(Data->ColliderRadius);
		
		// Setup AI
		AIControllerClass = Data->AI;

		if (Controller)
			Controller->Destroy();

		if (bRecreateAI && AIControllerClass)
		{
			// Copy paste from Pawn.cpp:339
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Instigator = GetInstigator();
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.OverrideLevel = GetLevel();
			SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save AI controllers into a map
			AController* NewController = GetWorld()->SpawnActor<AController>(AIControllerClass, GetActorLocation(), GetActorRotation(), SpawnInfo);
			if (NewController != nullptr)
			{
				// if successful will result in setting this->Controller 
				// as part of possession mechanics
				NewController->Possess(this);
			} else
			{
				ECO_LOG_ERROR(FString::Printf(TEXT("In Animal %ls of type %ls, failed to create AI!"), *GetName(), *Data->SpeciesName.ToString()));
			}
		}
	}
}
