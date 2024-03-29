﻿// copyright lololol

#include "Character/Animals/BaseAnimal.h"

#include "BrainComponent.h"
#include "Ecoscape.h"
#include "EcoscapeGameModeBase.h"
#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "EcoscapeStats.h"
#include "NavigationSystem.h"
#include "VorbisAudioInfo.h"
#include "Animation/AnimInstance.h"
#include "Async/Async.h"
#include "Character/EcoscapePlayerController.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Tasks/AITask_RunEQS.h"
#include "World/EcoscapeTerrain.h"

#if WITH_EDITOR
#include "MessageLogModule.h"
#endif

DECLARE_CYCLE_STAT(TEXT("Terrain/Animal: Check Happiness"), STAT_CheckHappiness, STATGROUP_EcoscapeTerrain);

TSharedPtr<FUpdateHappiness> ABaseAnimal::HappinessUpdateRunnable;

FUpdateHappiness::FUpdateHappiness()
{
	Thread = FRunnableThread::Create(this, TEXT("Animal Available Space Check"), 0, TPri_Lowest);
}

FUpdateHappiness::~FUpdateHappiness()
{
	if (Thread != nullptr)
	{
		Thread->Kill(true);
		delete Thread;
	}
}

bool FUpdateHappiness::Init()
{
	return true;
}

uint32 FUpdateHappiness::Run()
{
	bStopThread = false;

	if (bStopThread || !Animals.IsEmpty())
	{
		ABaseAnimal* Animal;
		Animals.Dequeue(Animal);

		// -------------
		// SIMPLE FLOOD FILL ALGORITHM
		// Copied from Wikipedia
		// -------------
		if (!Animal->GetTerrain())
		{
			FHappinessUpdateInfo UpdateInfo;
			UpdateInfo.PercentageOfHabitatAvailable = 0;
			Animal->OnHappinessUpdated.Broadcast(UpdateInfo);
			CurrentlyQueued.Remove(Animal);
			return 1;
		}
		float Width = Animal->GetTerrain()->Width, Height = Animal->GetTerrain()->Height;
		const int TheoreticalWalkablePointsNum = Animal->GetTerrain()->GetWalkableVertCount();
		const int StartIndex = Animal->GetTerrain()->GetClosestVertex(Animal->GetActorLocation());
		FVector2D Pos = Animal->GetTerrain()->GetVertexXY(StartIndex);
		const TArray Walkable = Animal->GetTerrain()->Walkable;

		TArray<int> WalkablePoints;
		if (!Walkable[StartIndex])
		{
			if (Walkable[StartIndex + 1])
				Pos = Animal->GetTerrain()->GetVertexXY(StartIndex + 1);
			else if (Walkable[StartIndex - 1])
				Pos = Animal->GetTerrain()->GetVertexXY(StartIndex - 1);
			else
			{
				FHappinessUpdateInfo UpdateInfo;
				UpdateInfo.PercentageOfHabitatAvailable = 0;
				UpdateInfo.Reachable = WalkablePoints;
				Animal->OnHappinessUpdated.Broadcast(UpdateInfo);
				CurrentlyQueued.Remove(Animal);
				return 0;
			}
		}
		TQueue<FVector4> PointsToSearch;
		PointsToSearch.Enqueue(FVector4(Pos.X, Pos.X, Pos.Y, 1));
		PointsToSearch.Enqueue(FVector4(Pos.X, Pos.X, Pos.Y - 1, -1));
		while (!PointsToSearch.IsEmpty())
		{
			FVector4 Current;
			PointsToSearch.Dequeue(Current);
			float X = Current.X;
			int I = (X * (Width + 1)) + Current.Z;
			if (!WalkablePoints.Contains(I) && I >= 0 && I < Walkable.Num() && Walkable[I])
			{
				I = ((X - 1) * (Width + 1)) + Current.Z;
				while (!WalkablePoints.Contains(I) && I >= 0 && I < Walkable.Num() && Walkable[I])
				{
					WalkablePoints.AddUnique(I);
					X--;

					I = ((X - 1) * (Width + 1)) + Current.Z;
				}
			}

			if (X < Current.X)
				PointsToSearch.Enqueue(FVector4(X, Current.X - 1, Current.Z - Current.W, -Current.W));
			while (Current.X <= Current.Y)
			{
				I = (Current.X * (Width + 1)) + Current.Z;
				while (!WalkablePoints.Contains(I) && I >= 0 && I < Walkable.Num() && Walkable[I])
				{
					WalkablePoints.AddUnique(I);
					Current.X++;
					PointsToSearch.Enqueue(FVector4(X, Current.X - 1, Current.Z + Current.W, Current.W));
					if (Current.X - 1 > Current.Y)
						PointsToSearch.Enqueue(
							FVector4(Current.Y + 1, Current.X - 1, Current.Z - Current.W, -Current.W));

					I = (Current.X * (Width + 1)) + Current.Z;
				}
				Current.X++;
				I = (Current.X * (Width + 1)) + Current.Z;
				while (Current.X < Current.Y && !(!WalkablePoints.Contains(I) && I >= 0 && I < Walkable.Num() &&
					Walkable[I]))
				{
					Current.X++;
					I = (Current.X * (Width + 1)) + Current.Z;
				}
				X = Current.X;
			}
		}

		FHappinessUpdateInfo UpdateInfo;
		UpdateInfo.PercentageOfHabitatAvailable = FMath::Clamp(
			static_cast<float>(WalkablePoints.Num()) / TheoreticalWalkablePointsNum, 0, 1);
		UpdateInfo.Reachable = WalkablePoints;
		Animal->OnHappinessUpdated.Broadcast(UpdateInfo);
		// AsyncTask(ENamedThreads::GameThread, [UpdateInfo, Animal] ()
		// 			{
		// 				Animal->OnHappinessUpdated.Broadcast(UpdateInfo); 
		// 			});

		CurrentlyQueued.Remove(Animal);
	}

	return 0;
}

void FUpdateHappiness::Exit()
{
}

void FUpdateHappiness::Stop()
{
	bStopThread = true;
}

void FUpdateHappiness::EnqueueAnimalForUpdate(ABaseAnimal* Animal, bool bRunIfNot)
{
	if (!CurrentlyQueued.Contains(Animal))
	{
		Animals.Enqueue(Animal);
		CurrentlyQueued.Add(Animal);
	}

	Run();
}

ABaseAnimal::ABaseAnimal()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->RotationRate.Yaw = 180;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Enable avoidance
	GetCharacterMovement()->bUseRVOAvoidance = true;

	GetMesh()->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	GetMesh()->SetCollisionResponseToChannel(ECC_BLOCKS_ITEM_PLACEMENT, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_BLOCKS_GROWTH, ECR_Block);

	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_Yes;
	GetMesh()->CanCharacterStepUpOn = ECB_Yes;

	Audio = CreateDefaultSubobject<UAudioComponent>("Audio");
	Audio->SetupAttachment(RootComponent);

	Outline = CreateDefaultSubobject<UOutlineComponent>("Outline");
	GetMesh()->ComponentTags.Add("Outline");
	Outline->RefreshOutlinedComponents();

	SicknessInteraction = CreateDefaultSubobject<USickAnimalInteractionComponent>("Sick Interaction");
}

void ABaseAnimal::BeginPlay()
{
	Super::BeginPlay();

	if (!HappinessUpdateRunnable)
		HappinessUpdateRunnable = MakeShared<FUpdateHappiness>();

	if (!AnimalData)
	{
		ECO_LOG_ERROR(FString::Printf(TEXT("Null AnimalData in %s"), *GetName()));
		Destroy();
		return;
	}

	SetAnimalData(AnimalData);

	// No idea why, but these timers are getting unset randomly.
	// FTimerHandle HappinessTest, HappinessUpdate;
	// GetWorld()->GetTimerManager().SetTimer(HappinessTest, FTimerDelegate::CreateUFunction(this, "UpdateHappiness"), 2, true, 0);
	// GetWorld()->GetTimerManager().SetTimer(HappinessUpdate, FTimerDelegate::CreateUFunction(this, "RecalculateHappiness"), 1, true, 0);

	FScriptDelegate Binding;
	Binding.BindUFunction(this, "OnReceiveHappinessUpdated");
	OnHappinessUpdated.AddUnique(Binding);

	if (AssociatedTerrain)
		SetTerrain(AssociatedTerrain);
}

void ABaseAnimal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead)
		return;

	if (Health > 0 && bIsTrapped)
	{
		Health -= 4 * DeltaSeconds;
		if (Health <= 0)
			DeathMessage = FText::FromString("was trapped.");
	}

	// Do hunger
	if (Health > 0 && Hunger <= 0 && !bIsEating)
	{
		Health -= 1 * DeltaSeconds; // Panda will die in ~60 seconds from no food
		if (Health <= 0)
			DeathMessage = FText::FromString("starved to death.");
	}
	else
		Hunger = FMath::Max(0, Hunger - (DeltaSeconds * AnimalData->HungerRate * (bIsSleeping ? 0.3f : 1)));

	if (Health > 0 && Thirst <= 0 && !bIsDrinking)
	{
		Health -= 2 * DeltaSeconds;
		if (Health <= 0)
			DeathMessage = FText::FromString("died of dehydration.");
	}
	else
		Thirst = FMath::Max(0, Thirst - (DeltaSeconds * AnimalData->ThirstRate * (bIsSleeping ? 0.3f : 1)));

	// Sickness
	if (Health > 0 && bIsSick && Medicine <= 0)
	{
		Health -= 2 * DeltaSeconds;
		if (Health <= 0)
			DeathMessage = FText::FromString("was sick.");
	}

	// Healing
	if (Hunger > 0 && Thirst > 0 && !bIsTrapped)
		Health = FMath::Min(AnimalData->BaseHealth, Health + (DeltaSeconds * OverallHappiness * 3));

	if (!bDead && Health <= 0)
	{
		Die();
		Cast<AAIController>(Controller)->GetBrainComponent()->StopLogic("Death");
		SetLifeSpan(5);
	}

	// DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 100),
	//                 FString::Printf(
	// 	                TEXT("HP: %f, Hunger: %f, Thirst: %f, Freedom: %f"), Health, Hunger, Thirst,
	// 	                PercentageOfHabitatAvailable), nullptr, FColor::White, DeltaSeconds);

	if (HappinessRecalcTimer > 0)
	{
		HappinessRecalcTimer -= DeltaSeconds;
		if (HappinessRecalcTimer <= 0)
			RecalculateHappiness();
	}

	FreedomCheckTimer -= DeltaSeconds;
	if (FreedomCheckTimer <= 0)
		UpdateHappiness();

	if (FoodWaterCheckTimer > 0)
	{
		FoodWaterCheckTimer -= GetWorld()->DeltaRealTimeSeconds;
		if (FoodWaterCheckTimer <= 0)
			CheckFoodWater();
	}

	SicknessInteraction->bCanInteract = bIsSick && Medicine <= 0;

	if (Medicine > 0)
	{
		if (bIsSick)
		{
			Medicine -= 0.05 * DeltaSeconds;
		}
		else Medicine = 0;
	}

	if (SicknessCheckTimer > 0)
	{
		SicknessCheckTimer -= DeltaSeconds;
		if (SicknessCheckTimer <= 0)
		{
			auto GameMode = AEcoscapeGameModeBase::GetEcoscapeBaseGameMode(GetWorld());
			if (bIsSick
				&& Medicine > 0)
			{
				if (FMath::FRand() <= 0.05f)
				{
					bIsSick = false;
					Medicine = 0;
				}
			}
			else if (!bIsSick
				&& GameMode->CanAnimalGetSick(this)
				&& FMath::FRand() <= AnimalData->SicknessChance * GameMode->GetAnimalSicknessModifier())
			{
				bIsSick = true;
				GameMode->GiveCodexEntry(UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld())->GetCodexEntry("CDX_Sickness"));
				GameMode->NotificationPanel->AddNotification(
					FText::FromString(FString::Printf(TEXT("%s is sick!"), *GivenName)),
					FText::FromString(
						FString::Printf(
							TEXT("Go to your %s and give them medicine."), *AssociatedTerrain->TerrainName)));
				RecalculateHappiness();
			}
			SicknessCheckTimer = 1;
		}
	}

	// Do sound
	if (AnimalData)
	{
		SoundTimer -= DeltaSeconds;
		if (SoundTimer <= 0)
		{
			Audio->Play();
			SoundTimer = FMath::FRandRange(AnimalData->SoundTimeRange.X, AnimalData->SoundTimeRange.Y);
		}
	}

	// Find target rotation
	FRotator CurrentRotation = GetMesh()->GetComponentRotation();
	double CurrentYaw = CurrentRotation.Yaw;

	if (GetCharacterMovement()->GetCurrentAcceleration().SizeSquared() < KINDA_SMALL_NUMBER)
	{
		// AI path following request can orient us in that direction (it's effectively an acceleration)
		if (GetCharacterMovement()->RequestedVelocity.SizeSquared() > KINDA_SMALL_NUMBER)
			TargetYaw = GetCharacterMovement()->RequestedVelocity.GetSafeNormal().Rotation().Yaw;
	}
	else
		TargetYaw = GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal().Rotation().Yaw;

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
	GetMesh()->SetWorldRotation(
		UKismetMathLibrary::RInterpTo_Constant(CurrentRotation, TargetRotation, DeltaSeconds, 15));

	FRotator CurrentRot = GetActorRotation();
	FRotator TargetRot = CurrentRot;
	TargetRot.Yaw = TargetYaw;
	SetActorRotation(UKismetMathLibrary::RInterpTo_Constant(CurrentRot, TargetRot, DeltaSeconds, 160));
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
		GetMesh()->SetRelativeRotation(Data->MeshRotation);
		GetMesh()->SetWorldScale3D(Data->MeshScale);

		// Setup capsule
		GetCapsuleComponent()->SetCapsuleHalfHeight(Data->ColliderHalfHeight);
		GetCapsuleComponent()->SetCapsuleRadius(Data->ColliderRadius);

		// Setup sound
		Audio->SetSound(Data->Sound);
		Audio->AttenuationSettings = Data->Sound->AttenuationSettings;
		SoundTimer = FMath::FRandRange(Data->SoundTimeRange.X, Data->SoundTimeRange.Y);

		// Setup EQS
		if (Data->FoodQuery)
			FoodRequest = FEnvQueryRequest(Data->FoodQuery, this);
		if (Data->WaterQuery)
			WaterRequest = FEnvQueryRequest(Data->WaterQuery, this);

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
			SpawnInfo.ObjectFlags |= RF_Transient; // We never want to save AI controllers into a map
			AController* NewController = GetWorld()->SpawnActor<AController>(
				AIControllerClass, GetActorLocation(), GetActorRotation(), SpawnInfo);
			if (NewController != nullptr)
			{
				// if successful will result in setting this->Controller 
				// as part of possession mechanics
				NewController->Possess(this);
			}
			else
			{
				ECO_LOG_ERROR(
					FString::Printf(TEXT("In Animal %ls of type %ls, failed to create AI!"), *GetName(), *Data->
						SpeciesName.ToString()));
			}
		}
	}
}

void ABaseAnimal::SetTerrain(AEcoscapeTerrain* Terrain)
{
	if (TerrainWalkabilityHandle.IsValid())
		AssociatedTerrain->WalkabilityUpdated.Remove(TerrainWalkabilityHandle);
	AssociatedTerrain = Terrain;
	if (AssociatedTerrain)
		AssociatedTerrain->WalkabilityUpdated.AddLambda([this] { bNeedsFreedomUpdate = true; });
}

ABaseAnimal* ABaseAnimal::SpawnAnimal(UObject* World, UAnimalData* Data, AEcoscapeTerrain* Terrain, FVector Position)
{
	const FTransform TF(Position);
	ABaseAnimal* Animal = World->GetWorld()->SpawnActorDeferred<ABaseAnimal>(Data->AnimalClass, TF);
	if (Animal)
	{
		Animal->AnimalData = Data;
		Animal->AssociatedTerrain = Terrain;
		Animal->FinishSpawning(TF);
	}
	return Animal;
}

void ABaseAnimal::UpdateHappiness()
{
	SCOPE_CYCLE_COUNTER(STAT_CheckHappiness);

	FreedomCheckTimer = 2;

	if (bNeedsFreedomUpdate)
	{
		HappinessUpdateRunnable->EnqueueAnimalForUpdate(this);
		bNeedsFreedomUpdate = false;
	}

	//
	// int WalkablePointsNum = AssociatedTerrain->GetWalkableVertCount();
	// int StartIndex = AssociatedTerrain->GetClosestVertex(GetActorLocation());
	// FVector2D Pos = AssociatedTerrain->GetVertexXY(StartIndex);
	// TArray<int> WalkablePoints;
	//
	// if (!AssociatedTerrain->IsVertWalkable(StartIndex))
	// 	return;
	//
	// TQueue<FVector2D> Queue;
	// Queue.Enqueue(Pos);
	//
	// while (!Queue.IsEmpty())
	// {
	// 	FVector2D Current;
	// 	Queue.Dequeue(Current);
	// 	float LX = Current.X;
	//
	// 	while (!WalkablePoints.Contains())
	// }

	// ----
	// NAV TEST 2
	// ----
	// UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	//
	// INavigationDataInterface* const NavData = NavSys->GetNavDataForActor(*GetOwner());
	//
	// const ARecastNavMesh* const RecastNavMesh = Cast<const ARecastNavMesh>(NavData);
	// 					
	// FRecastDebugGeometry NavMeshTileGeo;
	// NavMeshTileGeo.bGatherNavMeshEdges = true;
	//
	// TArray<FVector> PointsToGatherTiles = { GetActorLocation() };
	// TSet<int32> AddedTileIndices;
	//
	// for (const FVector& GatherPoint : PointsToGatherTiles)
	// {
	// 	int32 TileX, TileY;
	// 	RecastNavMesh->GetNavMeshTileXY(GatherPoint, TileX, TileY);
	//
	// 	TArray<int32> TileIndices;
	// 	RecastNavMesh->GetNavMeshTilesAt(TileX, TileY, TileIndices);
	//
	// 	for (int32 i = 0; i < TileIndices.Num(); ++i)
	// 	{
	// 		if (!AddedTileIndices.Contains(TileIndices[i]))
	// 		{
	// 			RecastNavMesh->GetDebugGeometry(NavMeshTileGeo, TileIndices[i]);
	// 			AddedTileIndices.Add(TileIndices[i]);
	// 		}
	// 	}
	// }
	//
	// for (int32 i = 0; i < NavMeshTileGeo.NavMeshEdges.Num(); i += 2)
	// {
	// 	DrawDebugLine(GetWorld(), NavMeshTileGeo.NavMeshEdges[i], NavMeshTileGeo.NavMeshEdges[i + 1], FColor::Red, false, 2, 0, 5);
	// }

	// ----
	// FLOOD FILL 1
	// ----


	// if (bDrawNav)
	// {
	// 	for (const auto& WalkablePoint : WalkablePoints)
	// 		DrawDebugSphere(GetWorld(), AssociatedTerrain->GetVertexPositionWorld(WalkablePoint), 30, 6, FColor::Red, false, 1, 0, 2);
	// }
}

void ABaseAnimal::OnReceiveHappinessUpdated(FHappinessUpdateInfo Info)
{
	PercentageOfHabitatAvailable = Info.PercentageOfHabitatAvailable;
	RecalculateHappiness();

	if (bDrawNav)
	{
		for (int Vert : Info.Reachable)
		{
			DrawDebugSphere(GetWorld(), AssociatedTerrain->GetVertexPositionWorld(Vert), 30, 6, FColor::Red, false, 2);
		}
	}
}

void ABaseAnimal::RecalculateHappiness()
{
	FreedomHappiness = FMath::Clamp(PercentageOfHabitatAvailable * 2.5, 0, 1);
	FoodHappiness = FMath::Clamp(FoodSourcesAvailable / 6, 0, 1);
	DrinkHappiness = FMath::Clamp(0.5 + DrinkSourcesAvailable, 0, 1);
	DiseaseHappiness = bIsSick ? 0.3 : 1;
	if (AssociatedTerrain)
		EnvironmentHappiness = FMath::Clamp(AssociatedTerrain->Diversity * 1.15, 0, 1);

	OverallHappiness = FMath::Clamp(FreedomHappiness, 0.25, 1)
		* FMath::Clamp(FoodHappiness, 0.2, 1)
		* FMath::Clamp(DrinkHappiness, 0.2, 1)
		* FMath::Clamp(DiseaseHappiness, 0.2, 1)
		* FMath::Clamp(EnvironmentHappiness, 0.25, 1);

	if (bHasHappinessOverride)
		OverallHappiness = HappinessOverride;

	AEcoscapeGameModeBase::GetEcoscapeBaseGameMode(GetWorld())->AnimalHappinessUpdated.
	                                                            Broadcast(this, OverallHappiness);

	HappinessRecalcTimer = 1;
}

void ABaseAnimal::CheckFoodWater()
{
	if (AnimalData->FoodQuery)
	{
		FoodRequest.Execute(EEnvQueryRunMode::AllMatching, FQueryFinishedSignature::CreateLambda(
			                    [this](TSharedPtr<FEnvQueryResult> Result)
			                    {
				                    FoodSourcesAvailable = Result->Items.Num();
				                    RecalculateHappiness();
			                    }));
	}

	if (AnimalData->WaterQuery)
	{
		WaterRequest.Execute(EEnvQueryRunMode::AllMatching, FQueryFinishedSignature::CreateLambda(
			                     [this](TSharedPtr<FEnvQueryResult> Result)
			                     {
				                     DrinkSourcesAvailable = Result->Items.Num();
				                     RecalculateHappiness();
			                     }));
	}

	FoodWaterCheckTimer = 5;
}

void ABaseAnimal::GiveMedicine()
{
	Medicine = 1;
}

void ABaseAnimal::Die()
{
	bDead = true;
	Health = 0;
	OnDeath.Broadcast();
	AEcoscapeGameModeBase::GetEcoscapeBaseGameMode(GetWorld())->AnimalDies.Broadcast(this);
}

// void ABaseAnimal::OnNavTest()
// {
// 	if (!bDrawNav)
// 		return;
// 	
// 	auto Nav = UNavigationSystemV1::GetCurrent(GetWorld());
// 	const auto NavData = Nav->GetMainNavData();
// 	ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);
// 	NavMesh->FindDistanceToWall()
//
// 	FVector Origin, Extents;
// 	GetActorBounds(true, Origin, Extents);
// 	auto Nearest = NavMesh->FindNearestPoly(Origin, Extents, NavMesh->GetDefaultQueryFilter());
//
// 	TArray<FVector> Points;
// 	TArray<NavNodeRef> VisitedNodes;
// 	NavSearch(NavMesh, Points, VisitedNodes, Nearest);
// }
//
// void ABaseAnimal::NavSearch(ARecastNavMesh* NavMesh, TArray<FVector>& Points, TArray<NavNodeRef>& Visited, NavNodeRef Nav)
// {
// 	if (Visited.Contains(Nav))
// 		return;
// 	Visited.Add(Nav);
// 	
// 	TArray<FNavigationPortalEdge> Edges;
// 	NavMesh->GetPolyEdges(Nav, Edges);
// 	for (const auto& Edge : Edges)
// 	{
// 		Points.Add(Edge.GetMiddlePoint());
// 		DrawDebugSphere(GetWorld(), Edge.GetMiddlePoint(), 30, 6, FColor::Red, false, 1, 0, 5);
// 	}
//
// 	TArray<FNavigationPortalEdge> Neighbours;
// 	NavMesh->GetPolyNeighbors(Nav, Neighbours);
// 	for (const auto& Neighbour : Neighbours)
// 		NavSearch(NavMesh, Points, Visited, Neighbour.ToRef);
// }
