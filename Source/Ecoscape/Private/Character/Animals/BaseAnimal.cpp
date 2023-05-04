// copyright lololol

#include "Character/Animals/BaseAnimal.h"

#include "Ecoscape.h"
#include "EcoscapeStatics.h"
#include "MessageLogModule.h"

ABaseAnimal::ABaseAnimal()
{
	PrimaryActorTick.bCanEverTick = true;

	AnimalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Animal Mesh"));
	AnimalMesh->SetCollisionResponseToChannel(ECC_BLOCKS_ITEM_PLACEMENT, ECR_Block);
	RootComponent = AnimalMesh;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
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
		AnimalMesh->SetSkeletalMesh(Data->Mesh);
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
