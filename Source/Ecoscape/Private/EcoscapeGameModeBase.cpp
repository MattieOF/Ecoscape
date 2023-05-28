// Copyright Epic Games, Inc. All Rights Reserved.

#include "EcoscapeGameModeBase.h"

#include "Ecoscape.h"
#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "Character/EcoscapeFPPlayerCharacter.h"
#include "Character/EcoscapePlayerController.h"
#include "Character/Animals/BaseAnimal.h"
#include "World/EcoscapeTerrain.h"

#if WITH_EDITOR
#include "MessageLogModule.h"
#endif

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

	NotificationPanel = CreateWidget<UNotificationPanel>(GetWorld(), NotificationsWidgetClass);
	if (!NotificationPanel)
		ECO_LOG_ERROR("Failed to create notification panel!");
	else
		NotificationPanel->AddToViewport();
	
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

			if (!Progression->Stages[i].Animal)
				continue;
			
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

void AEcoscapeGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CurrentWidget || !CurrentWidget->IsInViewport())
	{
		CurrentWidget = nullptr;
		if (WidgetQueue.Num() != 0)
		{
			CurrentWidget = WidgetQueue[0];
			WidgetQueue.RemoveAt(0);
			CurrentWidget->AddToViewport();
		}
	}

	if (CurrentProgressionStage <= Progression->Stages.Num() - 1)
	{
		const auto& Stage = Progression->Stages[CurrentProgressionStage];
		switch (Progression->Stages[CurrentProgressionStage].RequirementType)
		{
		case PRTAnimalHappiness:
			{
				if (CurrentAnimals.Contains(CurrentProgressionStage) 
					&& CurrentAnimals[CurrentProgressionStage] 
					&& Progression->Stages[CurrentProgressionStage].MinimumHappiness <= CurrentAnimals[CurrentProgressionStage]->OverallHappiness)
				{
					UnlockNextAnimal();
				}
			}
			break;
		case PRTHabitatDeadness:
			{
				UEcoscapeGameInstance* GI = UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld());
				const AEcoscapeTerrain* Terrain = GI->GetTerrain(Stage.Habitat);
				if (Terrain->Deadness <= Stage.MaxDeadness)
					UnlockNextAnimal();
			}
			break;
		case PRTHabitatDiversity:
			{
				UEcoscapeGameInstance* GI = UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld());
				const AEcoscapeTerrain* Terrain = GI->GetTerrain(Stage.Habitat);
				if (Terrain->Diversity >= Stage.MinDiversity)
					UnlockNextAnimal();
			}
			break;
		default:
			ECO_LOG_ERROR(FString::Printf(TEXT("Invalid progression requirement type %i!"), static_cast<int>(Progression->Stages[CurrentProgressionStage].RequirementType)));
			break;
		}
	}
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
	for (const auto& ArrayAnimal : CurrentAnimals)
	{
		if (ArrayAnimal.Value == Animal)
		{
			if (!Animal->bIsSick && SickAnimals.Contains(ArrayAnimal.Key))
				SickAnimals.Remove(ArrayAnimal.Key);
			else if (Animal->bIsSick && !SickAnimals.Contains(ArrayAnimal.Key))
				SickAnimals.Add(ArrayAnimal.Key);
		}
	}
}

void AEcoscapeGameModeBase::OnAnimalDies(ABaseAnimal* Animal)
{
	TArray<int> AnimalIndexes;
	CurrentAnimals.GetKeys(AnimalIndexes);
	for (const int i : AnimalIndexes)
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
	Stage = FMath::Clamp(Stage, 0, Progression->Stages.Num());

	if (Stage <= CurrentProgressionStage)
		return;
	
	CurrentProgressionStage = Stage;

	if (Stage != Progression->Stages.Num() && Progression->Stages[Stage].Animal)
	{
		UAnimalUnlockedWidget* AnimalUnlockedWidget = CreateWidget<UAnimalUnlockedWidget>(UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld()), AnimalUnlockedWidgetClass);
		AnimalUnlockedWidget->SetAnimal(Progression->Stages[CurrentProgressionStage].Animal, Progression->Stages[CurrentProgressionStage].Habitat);
		AddWidgetToQueue(AnimalUnlockedWidget);
	}

	// Ensure all codex entries are unlocked
	for (int i = 0; i < CurrentProgressionStage; i++)
	{
		for (UCodexEntry* Entry : Progression->Stages[i].UnlockedCodexEntries)
			GiveCodexEntry(Entry);
		if (Progression->Stages[i].Animal && Progression->Stages[i].Animal->RelevantCodexEntry)
			GiveCodexEntry(Progression->Stages[i].Animal->RelevantCodexEntry);	
	}
}

void AEcoscapeGameModeBase::GiveCodexEntry(UCodexEntry* CodexEntry)
{
	if (!UnlockedCodexEntries.Contains(CodexEntry))
	{
		UnlockedCodexEntries.Add(CodexEntry);
		Codex->UpdateEntries();
		if (CodexEntry->bShouldNotify)
			CodexFeed->AddUnlock(CodexEntry);
	}
}

bool AEcoscapeGameModeBase::CanAnimalGetSick(ABaseAnimal* Animal)
{
	for (const auto& ArrayAnimal : CurrentAnimals)
	{
		if (ArrayAnimal.Value == Animal)
		{
			if (CurrentProgressionStage >= Progression->Stages.Num() - 1)
				return true;

			return ArrayAnimal.Key < CurrentProgressionStage;			
		}
	}

	return false;
}

void AEcoscapeGameModeBase::StartNewSave()
{
	const auto GI = UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld());
	
	// Reset codex
	UnlockedCodexEntries.Empty();
	GiveCodexEntry(GI->GetCodexEntry("CDX_Welcome"));
	GiveCodexEntry(GI->GetCodexEntry("CDX_Controls"));
	GiveCodexEntry(GI->GetCodexEntry("CDX_KnownBugs"));
	GiveCodexEntry(GI->GetCodexEntry("CDX_Survey"));
	GiveCodexEntry(GI->GetCodexEntry("CDX_Credits"));

	// Reset progression
	CurrentProgressionStage = 0;
	
	// Destroy all current animals
	for (const auto& Animal : CurrentAnimals)
		Animal.Value->Destroy();
	CurrentAnimals.Empty();
	SickAnimals.Empty();

	// Regenerate terrains
	const auto& Terrains = GI->GetAllTerrains();
	for (AEcoscapeTerrain* Terrain : Terrains)
	{
		Terrain->Regenerate();
		if (Terrain->TerrainName == "Forest")
			AEcoscapePlayerController::GetEcoscapePlayerController(GetWorld())->GoToTerrain(Terrain);
	}
}

void AEcoscapeGameModeBase::AddWidgetToQueue(UUserWidget* Widget)
{
	WidgetQueue.Add(Widget);
}
