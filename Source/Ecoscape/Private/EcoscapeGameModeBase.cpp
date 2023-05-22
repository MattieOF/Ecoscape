// Copyright Epic Games, Inc. All Rights Reserved.

#include "EcoscapeGameModeBase.h"

#include "Ecoscape.h"
#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "MessageLogModule.h"
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

	PrimaryActorTick.bCanEverTick = true;
}

void AEcoscapeGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	Codex = CreateWidget<UCodexUI>(GetWorld(), CodexWidgetClass);
	if (!Codex)
		ECO_LOG_ERROR("Failed to create codex!");
	else
		Codex->AddToViewport();

	CodexFeed = CreateWidget<UCodexFeedUI>(GetWorld(), CodexFeedWidgetClass);
	if (!CodexFeed)
		ECO_LOG_ERROR("Failed to create codex feed!");
	else
		CodexFeed->AddToViewport();

	FScriptDelegate AnimalHappinessUpdatedDelegate;
	AnimalHappinessUpdatedDelegate.BindUFunction(this, "OnAnimalHappinessUpdated");
	AnimalHappinessUpdated.Add(AnimalHappinessUpdatedDelegate);
	
	FScriptDelegate AnimalDiesDelegate;
	AnimalDiesDelegate.BindUFunction(this, "OnAnimalDies");
	AnimalDies.Add(AnimalDiesDelegate);

	FTimerHandle Handle;
	FTimerDelegate SpawnTimerDelegate;
	SpawnTimerDelegate.BindLambda([this]
	{
		for (int i = 0; i <= CurrentProgressionStage; i++)
		{
			if (CurrentProgressionStage > Progression->Stages.Num() - 1)
				break;

			if (!CurrentAnimals.Contains(i))
			{
				AEcoscapeTerrain* Terrain = UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld())->GetTerrain(Progression->Stages[i].Habitat);
				if (!Terrain)
				{
					UE_LOG(LogEcoscape, Error, TEXT("Invalid terrain %s on progression item %i"), *Progression->Stages[i].Habitat, i);
					break;
				}

				// TODO: Require player to be on another habitat? Or be looking elsewhere?
				
				FVector Spawn = Terrain->FindSpawnPoint();
				ABaseAnimal* SpawnedAnimal = ABaseAnimal::SpawnAnimal(GetWorld(), Progression->Stages[i].Animal, Terrain, Spawn + FVector(0, 0, 100));
				CurrentAnimals.Add(i, SpawnedAnimal);
			}
		}
	});
	GetWorldTimerManager().SetTimer(Handle, SpawnTimerDelegate, 7, true, 0);
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

void AEcoscapeGameModeBase::OnAnimalHappinessUpdated(ABaseAnimal* Animal, float NewHappiness)
{
	if (CurrentAnimals.Contains(CurrentProgressionStage) 
	    && CurrentAnimals[CurrentProgressionStage] == Animal
	    && Progression->Stages[CurrentProgressionStage].MinimumHappiness <= NewHappiness)
	{
		UnlockNextAnimal();
	}
}

void AEcoscapeGameModeBase::OnAnimalDies(ABaseAnimal* Animal)
{
	TArray<int> AnimalIndexes;
	CurrentAnimals.GetKeys(AnimalIndexes);
	for (int i : AnimalIndexes)
	{
		if (CurrentAnimals[i] == Animal)
		{
			CurrentAnimals.Remove(i);
			break;
		}
	}
}

void AEcoscapeGameModeBase::UnlockNextAnimal()
{
	UnlockStage(CurrentProgressionStage + 1);
}

void AEcoscapeGameModeBase::UnlockStage(int Stage)
{
	Stage = FMath::Clamp(Stage, 0, Progression->Stages.Num() - 1);

	if (Stage <= CurrentProgressionStage)
		return;
	
	CurrentProgressionStage = Stage;
	UAnimalUnlockedWidget* AnimalUnlockedWidget = CreateWidget<UAnimalUnlockedWidget>(GetWorld(), AnimalUnlockedWidgetClass);
	AnimalUnlockedWidget->SetAnimal(Progression->Stages[CurrentProgressionStage].Animal, Progression->Stages[CurrentProgressionStage].Habitat);
	AnimalUnlockedWidget->AddToViewport();
}

void AEcoscapeGameModeBase::GiveCodexEntry(UCodexEntry* CodexEntry)
{
	if (!UnlockedCodexEntries.Contains(CodexEntry))
	{
		UnlockedCodexEntries.Add(CodexEntry);
		Codex->UpdateEntries();
		CodexFeed->AddUnlock(CodexEntry);
	}
}
