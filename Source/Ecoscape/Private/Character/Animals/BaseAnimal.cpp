// copyright lololol

#include "Character/Animals/BaseAnimal.h"

#include "Ecoscape.h"
#include "EcoscapeStatics.h"
#include "EcoscapeStats.h"
#include "MessageLogModule.h"
#include "NavigationSystem.h"
#include "Animation/AnimInstance.h"
#include "Character/EcoscapePlayerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "World/EcoscapeTerrain.h"

DECLARE_CYCLE_STAT(TEXT("Terrain/Animal: Check Happiness"), STAT_CheckHappiness, STATGROUP_EcoscapeTerrain);

TSharedPtr<FUpdateHappiness> ABaseAnimal::HappinessUpdateRunnable;

FUpdateHappiness::FUpdateHappiness()
{
	Thread = FRunnableThread::Create( this, TEXT("Animal Available Space Check") );
}

FUpdateHappiness::~FUpdateHappiness()
{
	if ( Thread != nullptr )
	{
		Thread->Kill( true );
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
		// Takes about ~30ms per animal. Pretty slow, so we do it on a seperate thread.
		// TODO: Could be optimised by updating walkability values when fences or items are placed and then just doing a flood fill on some booleans
		// -------------
		if (!Animal->AssociatedTerrain)
			return 1;
		const int TheoreticalWalkablePointsNum = Animal->AssociatedTerrain->GetWalkableVertCount();
		const int StartIndex = Animal->AssociatedTerrain->GetClosestVertex(Animal->GetActorLocation());
		const FVector2D Pos = Animal->AssociatedTerrain->GetVertexXY(StartIndex);
		TArray<int> WalkablePoints;
		if (!Animal->AssociatedTerrain->IsVertWalkable(StartIndex))
			return 0;
		TQueue<FVector4> PointsToSearch;
		PointsToSearch.Enqueue(FVector4(Pos.X, Pos.X, Pos.Y, 1));
		PointsToSearch.Enqueue(FVector4(Pos.X, Pos.X, Pos.Y - 1, -1));
		while (!PointsToSearch.IsEmpty())
		{
			FVector4 Current;
			PointsToSearch.Dequeue(Current);
			float X = Current.X;
			int I = Animal->AssociatedTerrain->GetVertexIndex(X, Current.Z);
			if (!WalkablePoints.Contains(I) && Animal->AssociatedTerrain->IsVertWalkable(I))
			{
				I = Animal->AssociatedTerrain->GetVertexIndex(X - 1, Current.Z);
				while (!WalkablePoints.Contains(I) && Animal->AssociatedTerrain->IsVertWalkable(I))
				{
					WalkablePoints.AddUnique(I);
					X--;
				
					I = Animal->AssociatedTerrain->GetVertexIndex(X - 1, Current.Z);
				}
			}
		
			if (X < Current.X)
				PointsToSearch.Enqueue(FVector4(X, Current.X - 1, Current.Z - Current.W, -Current.W));
			while (Current.X <= Current.Y)
			{
				I = Animal->AssociatedTerrain->GetVertexIndex(Current.X,Current.Z);
				while (!WalkablePoints.Contains(I) && Animal->AssociatedTerrain->IsVertWalkable(I))
				{
					WalkablePoints.AddUnique(I);
					Current.X++;
					PointsToSearch.Enqueue(FVector4(X, Current.X - 1, Current.Z + Current.W, Current.W));
					if (Current.X - 1 > Current.Y)
						PointsToSearch.Enqueue(FVector4(Current.Y + 1, Current.X - 1, Current.Z - Current.W, -Current.W));
				
					I = Animal->AssociatedTerrain->GetVertexIndex(Current.X,Current.Z);
				}
				Current.X++;
				I = Animal->AssociatedTerrain->GetVertexIndex(Current.X, Current.Z);
				while (Current.X < Current.Y && !(!WalkablePoints.Contains(I) && Animal->AssociatedTerrain->IsVertWalkable(I)))
				{
					Current.X++;
					I = Animal->AssociatedTerrain->GetVertexIndex(Current.X, Current.Z);
				}
				X = Current.X; 
			}
		}

		FHappinessUpdateInfo UpdateInfo;
		UpdateInfo.PercentageOfHabitatAvailable = static_cast<float>(WalkablePoints.Num()) / TheoreticalWalkablePointsNum;
		Animal->OnHappinessUpdated.Broadcast(UpdateInfo);

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

	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_Yes;
	GetMesh()->CanCharacterStepUpOn = ECB_Yes;
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

	FTimerHandle NavTest;
	GetWorld()->GetTimerManager().SetTimer(NavTest, FTimerDelegate::CreateUFunction(this, "UpdateHappiness"), 2, true, 0);
}

void ABaseAnimal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Do hunger
	if (Hunger <= 0 && !bIsEating)
		Health -= 1 * DeltaSeconds; // Panda will die in ~60 seconds from no food
	else
		Hunger = FMath::Max(0, Hunger - DeltaSeconds * AnimalData->HungerRate);

	if (Thirst <= 0 && !bIsDrinking)
		Health -= 2 * DeltaSeconds;
	else
		Thirst = FMath::Max(0, Thirst - DeltaSeconds * AnimalData->ThirstRate);

	if (Health <= 0)
	{
		OnDeath.Broadcast();
		Destroy();
	}

	DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 100),
	                 FString::Printf(TEXT("HP: %f, Hunger: %f, Thirst: %f"), Health, Hunger, Thirst), nullptr, FColor::White, DeltaSeconds);

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
	GetMesh()->SetWorldRotation(UKismetMathLibrary::RInterpTo_Constant(CurrentRotation, TargetRotation, DeltaSeconds, 15));

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

ABaseAnimal* ABaseAnimal::SpawnAnimal(UObject* World, UAnimalData* Data, AEcoscapeTerrain* Terrain, FVector Position)
{
	const FTransform TF(Position);
	ABaseAnimal* Animal = World->GetWorld()->SpawnActorDeferred<ABaseAnimal>(ABaseAnimal::StaticClass(), TF);
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

	HappinessUpdateRunnable->EnqueueAnimalForUpdate(this);
	
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
