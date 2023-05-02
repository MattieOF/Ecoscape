// copyright lololol

#include "World/EcoscapeTerrain.h"

#include "EcoscapeGameInstance.h"
#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "EcoscapeStats.h"
#include "KismetProceduralMeshLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/FileHelper.h"
#include "Serialization/BufferArchive.h"
#include "World/FastNoise.h"
#include "World/Fence/Fence.h"
#include "World/Fence/ProceduralFence.h"
#include "World/Fence/ProceduralFenceMesh.h"

DECLARE_CYCLE_STAT(TEXT("Terrain: Generate"), STAT_GenTerrain, STATGROUP_EcoscapeTerrain);
DECLARE_CYCLE_STAT(TEXT("Terrain: Get Nearest Vertex"), STAT_GetNearestVert, STATGROUP_EcoscapeTerrain);
DECLARE_CYCLE_STAT(TEXT("Terrain: Add Vertex Color"), STAT_AddVertColor, STATGROUP_EcoscapeTerrain);
DECLARE_CYCLE_STAT(TEXT("Terrain: Get Verticies In Sphere"), STAT_VertsInSphere, STATGROUP_EcoscapeTerrain);

AEcoscapeTerrain::AEcoscapeTerrain()
{
	PrimaryActorTick.bCanEverTick = false;

	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	ProceduralMeshComponent->SetupAttachment(GetRootComponent());
	ProceduralMeshComponent->SetCollisionResponseToChannel(ECC_ITEM_PLACEABLE_ON, ECR_Block);
}

void AEcoscapeTerrain::BeginPlay()
{
	Super::BeginPlay();

	Regenerate();
}

void AEcoscapeTerrain::Tick(float DeltaSeconds)
{
}

void AEcoscapeTerrain::SerialiseTerrain(FArchive& Archive)
{
	// Save/load mesh information
	Archive << Verticies;
	Archive << Triangles;
	Archive << UV0;
	Archive << VertexColors;
	Archive << Normals;
	Archive << Tangents;
	Archive << ColorOffsets;

	// Save/load placed items
	int Count = PlacedItems.Num();
	Archive << Count;

	if (Archive.IsSaving())
	{
		for (APlacedItem* Item : PlacedItems)
		{
			FTransform Transform = Item->GetActorTransform();
			Archive << Transform;
			FString ItemName = Item->GetItemData()->GetName();
			Archive << ItemName;
		}
	} else
	{
		// Destroy all placed items and empty array
		for (APlacedItem* Item : PlacedItems)
			Item->Destroy();
		PlacedItems.Empty();

		for (AProceduralFenceMesh* Fence : PlacedFences)
			Fence->Destroy();
		PlacedFences.Empty();

		for (const auto& Detail : DetailActors)
			Detail.Actor->Destroy();
		DetailActors.Empty();

		PlacedItems.Reserve(Count);

		UEcoscapeGameInstance* GameInstance = UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld());
		for (int i = 0; i < Count; i++)
		{
			// Get serialised values
			FTransform Transform;
			FString    Item;
			Archive << Transform;
			Archive << Item;

			if (!GameInstance->ItemTypes.Contains(Item))
			{
				UE_LOG(LogEcoscape, Error, TEXT("Non-existent item type %s found in terrain save!"), *Item);
				continue;
			}
			UPlaceableItemData* ItemData = GameInstance->ItemTypes[Item];
			
			APlacedItem* NewItem = GetWorld()->SpawnActor<APlacedItem>(ItemData->PlacedItemClass, Transform);
			NewItem->SetItemData(ItemData);
			NewItem->AssociatedTerrain = this; 
			PlacedItems.Add(NewItem);
		}
	}

	int FenceCount = PlacedFences.Num();
	Archive << FenceCount;
	if (Archive.IsSaving())
	{
		for (AProceduralFenceMesh* Fence : PlacedFences)
		{
			FVector Loc = Fence->GetActorLocation();
			Archive << Loc;
			Fence->SerialiseFence(Archive);
		}
	} else
	{
		for (int i = 0; i < FenceCount; i++)
		{
			FVector Loc;
			Archive << Loc;
			AProceduralFenceMesh* Fence = GetWorld()->SpawnActor<AProceduralFenceMesh>(FenceClass, Loc, FRotator::ZeroRotator);
			Fence->SerialiseFence(Archive);
			PlacedFences.Add(Fence);
		}
	}

	int DetailActorCount = DetailActors.Num();
	Archive << DetailActorCount;
	if (Archive.IsSaving())
	{
		for (auto& DetailActor : DetailActors)
			Archive << DetailActor;
	} else
	{
		DetailActors.Reserve(Count);
		for (int i = 0; i < Count; i++)
		{
			FTerrainDetailActor DetailInfo;
			Archive << DetailInfo;
			const UPlaceableItemData* Item = UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld())->ItemTypes[DetailInfo.ItemName];
			AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(DetailInfo.Pos, FRotator::ZeroRotator);
			MeshActor->SetMobility(EComponentMobility::Movable);
			MeshActor->GetStaticMeshComponent()->SetStaticMesh(Item->Mesh);
			MeshActor->SetFolderPath("ExteriorDetail/");
			MeshActor->SetMobility(EComponentMobility::Static);
			DetailInfo.Actor = MeshActor;
			DetailActors.Add(DetailInfo);
		}
	}
}

bool AEcoscapeTerrain::SerialiseTerrainToFile(FString Filename)
{
	// Serialise with buffer archive
	FBufferArchive BinarySaveArchive;
	SerialiseTerrain(BinarySaveArchive);

	// Save to disk
	const FString Path = *FString::Printf(TEXT("%lsSaves/%s"), *FPaths::ProjectSavedDir(), *Filename);
	const bool SaveResult = FFileHelper::SaveArrayToFile(BinarySaveArchive, *Path);
	if (!SaveResult)
	{
		UE_LOG(LogEcoscape, Error, TEXT("Failed to save terrain to %s!"), *Path);
		return false;
	}
	
	// Empty buffer
	BinarySaveArchive.FlushCache();
	BinarySaveArchive.Empty(); 
	return true;
}

bool AEcoscapeTerrain::DeserialiseTerrainFromFile(FString Filename)
{
	TArray<uint8> BinaryArray;

	// Load and verify file
	const FString Path = *FString::Printf(TEXT("%lsSaves/%ls"), *FPaths::ProjectSavedDir(), *Filename);
	const bool LoadFileResult = FFileHelper::LoadFileToArray(BinaryArray, *Path);
	if (!LoadFileResult)
	{
		UE_LOG(LogEcoscape, Error, TEXT("In Ecoscape Terrain loader, failed to open file %s!"), *Path);
		return false;
	}
	if (BinaryArray.Num() <= 0)
	{
		UE_LOG(LogEcoscape, Error, TEXT("In Ecoscape Terrain loader, file %s is empty!"), *Path);
		return false;
	}

	// Create binary loader
	FMemoryReader BinaryLoader = FMemoryReader(BinaryArray, true);
	BinaryLoader.Seek(0);
	
	// Do the loading
	ResetMeshData();
	SerialiseTerrain(BinaryLoader);
	GenerateFence();

	// Recreate terrain from loaded data
	CreateMesh();

	// Empty buffer
	BinaryLoader.FlushCache();
	BinaryArray.Empty();
	BinaryLoader.Close();

	return true;
}

void AEcoscapeTerrain::ResetMeshData()
{	
	Verticies.Empty();
	Triangles.Empty();
	UV0.Empty();
	Normals.Empty();
	Tangents.Empty();
	VertexColors.Empty();
	ColorOffsets.Empty();
}

void AEcoscapeTerrain::GenerateVerticies()
{
	int VertCount = 0;
	for (int x = 0; x <= Width; x++)
	{
		for (int y = 0; y <= Height; y++)
		{
			VertCount++;
			// TODO: ensure that it's never an integer
			float Z = 0;
			for (const auto& [NoiseType, CoordinateScale, HeightScale, Seed, Offset] : NoiseLayers)
			{
				switch (NoiseType)
				{
				case ENTPerlin:
					Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
					break;
				case ENTSimplex:
					Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
					break;
				default:
					UE_LOG(LogEcoscape, Error, TEXT("Invalid noise type: %i"), static_cast<int>(NoiseType));
					break;
				}

				Noise.SetSeed(Seed);

				Z += Noise.GetNoise(x * CoordinateScale, y * CoordinateScale) * HeightScale;
			}

			// Add exterior height offset
			const int Distance = FMath::Min(FMath::Min(x, y), FMath::Min(Width - x, Height - y));
			const float Alpha = 1 - (static_cast<float>(Distance - 3) / static_cast<float>(ExteriorTileCount - 3));
			Z += FMath::Lerp(0, ExteriorHeightOffset, FMath::Clamp(Alpha, 0, 1));

			AverageHeight += Z;
			HighestHeight = FMath::Max(HighestHeight, Z);
			LowestHeight = FMath::Min(LowestHeight, Z);
			
			Verticies.Add(FVector(x * Scale, y * Scale, Z));
			UV0.Add(FVector2D(x * UVScale, y * UVScale));
			
			Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
			Noise.SetSeed(ColorOffsetSeed);
			
			float Value = Noise.GetNoise((x + 0.1f) * 5.0f, (y + 0.1f) * 5.0f);
			Value = (Value + 1) / 2;
			Value = FMath::Lerp(ColorOffsetRange.X, ColorOffsetRange.Y, Value);
			FVector Offset = FVector(Value);
			ColorOffsets.Add(Offset);
			
			FColor VertexColor = UKismetMathLibrary::LinearColorLerp(FloorColour, ExteriorColour, FMath::Clamp(Alpha * 7, 0, 1)).ToFColor(false);
			VertexColor = UEcoscapeStatics::AddToColor(VertexColor, Offset);
			VertexColors.Add(VertexColor);
		}
	}

	AverageHeight /= VertCount;
}

void AEcoscapeTerrain::GenerateIndicies()
{
	int Vertex = 0;

	for (int X = 0; X < Width; ++X)
	{
		for (int Y = 0; Y < Height; ++Y)
		{
			Triangles.Add(Vertex); // Bottom left corner
			Triangles.Add(Vertex + 1); // Bottom right corner
			Triangles.Add(Vertex + Height + 1); // Top left corner
			Triangles.Add(Vertex + 1); // Bottom right corner
			Triangles.Add(Vertex + Height + 2); // Top right corner
			Triangles.Add(Vertex + Height + 1); // Top left corner

			++Vertex;
		}
		++Vertex;
	}
}

void AEcoscapeTerrain::GenerateNormals()
{
	Normals.Init(FVector::ZeroVector, Verticies.Num());

	for (int i = 0; i <  Triangles.Num(); i += 3)
	{
		const FVector P = FVector::CrossProduct(Verticies[Triangles[i + 1]] - Verticies[Triangles[i]], Verticies[Triangles[i + 2]] - Verticies[Triangles[i]]);
		Normals[Triangles[i]] += P;
		Normals[Triangles[i + 1]] += P;
		Normals[Triangles[i + 2]] += P;
	}

	for (int i = 0; i < Normals.Num(); i++)
	{
		Normals[i].Normalize();
		Normals[i] = -Normals[i];
	}
}

void AEcoscapeTerrain::GenerateFence()
{
	// This fence impl stuff is just for demonstration purposes.
#if WITH_EDITOR
	switch (FenceImplementation)
	{
	case EFIGeoScript:
		{
			int X = 3, Y = 3;
			AProceduralFence* Fence = GetWorld()->SpawnActor<AProceduralFence>(GetVertexPositionWorld(GetVertexIndex(X, Y)), FRotator::ZeroRotator);
			Fence->SplineComponent->ClearSplinePoints();
			Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));

			X += 2;
			for (; X < Width - 3; X += 2)
				Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
			for (; Y < Height - 3; Y += 2)
				Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
			for (; X > 3; X -= 2)
				Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
			for (; Y > 3; Y -= 2)
				Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));

			X = 3;
			Y = 3;
			Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));	
	
			Fence->Generate();
		}
		break;
	case EFISplineMesh:
		{
			int X = 3, Y = 3;
			AFence* Fence = GetWorld()->SpawnActor<AFence>(GetVertexPositionWorld(GetVertexIndex(X, Y)), FRotator::ZeroRotator);
			Fence->Spline->ClearSplinePoints();
			Fence->Spline->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));

			X += 2;
			for (; X < Width - 3; X += 2)
				Fence->Spline->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
			for (; Y < Height - 3; Y += 2)
				Fence->Spline->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
			for (; X > 3; X -= 2)
				Fence->Spline->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
			for (; Y > 3; Y -= 2)
				Fence->Spline->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));

			X = 3;
			Y = 3;
			Fence->Spline->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));	
	
			Fence->GenerateMesh();
		}
		break;
	case EFIProc:	
		{
#endif
			if (FenceMesh)
				FenceMesh->Destroy();
	
			int X = ExteriorTileCount + 1, Y = ExteriorTileCount + 1;
			FenceMesh = GetWorld()->SpawnActor<AProceduralFenceMesh>(FenceClass, GetVertexPositionWorld(GetVertexIndex(X, Y)), FRotator::ZeroRotator);
			FenceMesh->AssociatedTerrain = this;
			FenceMesh->bDestroyable = false;
			FenceMesh->Outline->DestroyComponent();
			FenceMesh->SplineComponent->ClearSplinePoints();
			FenceMesh->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));

			X += 2;
			for (; X < Width - ExteriorTileCount - 1; X += 2)
				FenceMesh->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
			for (; Y < Height - ExteriorTileCount - 1; Y += 2)
				FenceMesh->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
			for (; X > ExteriorTileCount + 1; X -= 2)
				FenceMesh->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
			for (; Y > ExteriorTileCount + 1; Y -= 2)
				FenceMesh->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));

			X = ExteriorTileCount + 1;
			Y = ExteriorTileCount + 1;
			FenceMesh->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));	
	
			FenceMesh->Regenerate();
#if WITH_EDITOR
		}
		break;
	default: break;
	}
#endif
}

// TODO: This function SUCKS
// Look into converting to using instanced static meshes: https://docs.unrealengine.com/4.27/en-US/BlueprintAPI/Components/InstancedStaticMesh/
void AEcoscapeTerrain::GenerateExteriorDetail()
{
	FVector TerrainLoc = GetActorLocation();
	
	for (int i = 0; i < 600; i++)
	{
		float X, Y;

		if (FMath::FRand() < 0.5)
		{
			// X is long side
			X = FMath::RandRange(static_cast<float>(0), Width * Scale);
			Y = FMath::RandRange(static_cast<float>(0), ExteriorTileCount * Scale);
			if (FMath::FRand() < 0.5)
				Y = (Height * Scale) - Y; 
		} else
		{
			// Y is long side
			Y = FMath::RandRange(static_cast<float>(0), Height * Scale);
			X = FMath::RandRange(static_cast<float>(0), ExteriorTileCount * Scale);
			if (FMath::FRand() < 0.5)
				X = (Width * Scale) - X;
		}

		X += TerrainLoc.X;
		Y += TerrainLoc.Y;

		FHitResult Hit;
		const FVector RayStart = FVector(X, Y, GetHighestHeight() + 50);
		const FVector RayEnd = FVector(X, Y, GetLowestHeight() - 50);
		if (GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_ITEM_PLACEABLE_ON))
		{
			UPlaceableItemData* Item = ExteriorItems[FMath::RandRange(0, ExteriorItems.Num() - 1)];
			AStaticMeshActor* Mesh = GetWorld()->SpawnActor<AStaticMeshActor>(Hit.ImpactPoint, FRotator::ZeroRotator);
			Mesh->SetMobility(EComponentMobility::Movable);
			Mesh->GetStaticMeshComponent()->SetStaticMesh(Item->Mesh);
			Mesh->AddActorLocalOffset(FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(Mesh) + Item->ZOffset));
			Mesh->SetFolderPath("ExteriorDetail/");

			if (bDoRotationForExteriorItems)
			{
				auto Rotation = UKismetMathLibrary::Conv_VectorToRotator(Hit.ImpactNormal);
				FVector UpVector = Mesh->GetActorUpVector();
				FVector NormalVector = UKismetMathLibrary::VLerp(UpVector, Hit.ImpactNormal, 0.4f);
				FVector RotationAxis = FVector::CrossProduct(UpVector, NormalVector);
				RotationAxis.Normalize();
				float DotProduct = FVector::DotProduct(UpVector, NormalVector);
				float RotationAngle = acosf(DotProduct);
				FQuat Quat = FQuat(RotationAxis, RotationAngle);
				FQuat RootQuat = Mesh->GetActorQuat();
				FQuat NewQuat = Quat * RootQuat;
				Rotation = NewQuat.Rotator();
				Mesh->SetActorRotation(Rotation);
			}
			
			Mesh->SetMobility(EComponentMobility::Static);
			
			DetailActors.Add({Item->GetName(), Mesh->GetActorLocation(), Mesh});
		}
	}
}

void AEcoscapeTerrain::CreateMesh() const
{
	ProceduralMeshComponent->CreateMeshSection(0, Verticies, Triangles, Normals, UV0, VertexColors, Tangents, true);
	ProceduralMeshComponent->SetMaterial(0, Material);
}

void AEcoscapeTerrain::CalculateVertColour(int Index, bool Flush)
{
	const FVector Position = GetVertexPositionWorld(Index);

	const auto Coordinate = GetVertexXY(Index);
	const int Distance = FMath::Min(FMath::Min(Coordinate.X, Coordinate.Y), FMath::Min(Width - Coordinate.X, Height - Coordinate.Y));
	const float Alpha = 1 - (static_cast<float>(Distance - 3) / static_cast<float>(ExteriorTileCount - 3));
	VertexColors[Index] = UKismetMathLibrary::LinearColorLerp(FloorColour, ExteriorColour, FMath::Clamp(Alpha * 7, 0, 1)).ToFColor(false);
	
	for (APlacedItem* Item : PlacedItems)
	{
		const float DistanceSquared = FVector::DistSquared(Position, Item->GetActorLocation());
		const auto Data = Item->GetItemData();
		// Only using the X scale here. Assumes uniform scale
		const FColor Remainder = Data->MaxLandColour.ToFColor(false) - VertexColors[Index];
		const FColor Offset = UEcoscapeStatics::ClampColor(
			Data->LandColour.ToFColor(false) * FMath::Clamp(1 - (DistanceSquared / Data->ColourRangeSquared), 0, 1),
			FColor(0, 0, 0), Remainder);
		VertexColors[Index] += Offset;
	}
	VertexColors[Index] = UEcoscapeStatics::AddToColor(VertexColors[Index], ColorOffsets[Index]);
	
	if (Flush)
		FlushMesh();
}

// TODO: Maybe add a faster approximate function?
FVector AEcoscapeTerrain::GetCenterPosition()
{
	const FVector Center = GetActorLocation() + FVector((Width * Scale) / 2, (Height * Scale) / 2, 3000);
	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, Center, Center + (FVector::DownVector * 4000), ECC_WorldStatic);
	return Hit.ImpactPoint;
}

void AEcoscapeTerrain::GetXYBounds(FVector2D& Min, FVector2D& Max)
{
	FVector Loc = GetActorLocation();
	Min = FVector2D(Loc.X, Loc.Y);
	Loc += FVector(Width * Scale, Height * Scale, 0);
	Max = FVector2D(Loc.X, Loc.Y);
}

int AEcoscapeTerrain::GetClosestVertex(FVector Position)
{
	SCOPE_CYCLE_COUNTER(STAT_GetNearestVert);
	
	const FVector ActorLoc = GetActorLocation();
	const float OriginX = ActorLoc.X;
	const float EndX = OriginX + (Width * Scale);
	const float OriginY = ActorLoc.Y;
	const float EndY = OriginY + (Height * Scale);

	// Using these two lines would be shorter, but I think slower, and this is quite a hot path
	// const float XAlpha = FMath::GetMappedRangeValueUnclamped(FVector2D(ActorLoc.X, ActorLoc.X + (Width * Scale)), FVector2D(0, 1), Position.X);
	// const float YAlpha = FMath::GetMappedRangeValueUnclamped(FVector2D(ActorLoc.Y, ActorLoc.Y + (Height * Scale)), FVector2D(0, 1), Position.Y);
	const float XAlpha = FMath::Clamp((Position.X - OriginX) / (EndX - OriginX), 0, 1);
	const float YAlpha = FMath::Clamp((Position.Y - OriginY) / (EndY - OriginY), 0, 1);

	const int X = FMath::RoundToInt(Width  * XAlpha);
	const int Y = FMath::RoundToInt(Height * YAlpha);

	return GetVertexIndex(X, Y);
}

bool AEcoscapeTerrain::IsPositionWithinPlayableSpace(FVector Position)
{
	const FVector Loc = GetActorLocation();
	return FMath::IsWithin(Position.X, Loc.X + (ExteriorTileCount * Scale), Loc.X + (Width * Scale) - (ExteriorTileCount * Scale))
		&& FMath::IsWithin(Position.Y, Loc.Y + (ExteriorTileCount * Scale), Loc.Y + (Height * Scale) - (ExteriorTileCount * Scale));
}

AProceduralFenceMesh* AEcoscapeTerrain::CreateFence(FVector2D Start, FVector2D End, int Step)
{
	const int LowX  = Start.X < End.X ? Start.X : End.X;
	const int LowY  = Start.Y < End.Y ? Start.Y : End.Y;
	const int HighX = Start.X > End.X ? Start.X : End.X;
	const int HighY = Start.Y > End.Y ? Start.Y : End.Y;
	
	int X = LowX, Y = LowY;
	AProceduralFenceMesh* Fence = GetWorld()->SpawnActor<AProceduralFenceMesh>(FenceClass, GetVertexPositionWorld(GetVertexIndex(X, Y)), FRotator::ZeroRotator);
	Fence->bShouldGenerateGate = true;
	Fence->AssociatedTerrain = this;
	Fence->SplineComponent->ClearSplinePoints();
	Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));

	for (; X < HighX; X = FMath::Min(HighX, X + Step))
		Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
	for (; Y < HighY; Y = FMath::Min(HighY, Y + Step))
		Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
	for (; X > LowX; X = FMath::Max(LowX, X - Step))
		Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));
	for (; Y > LowY; Y = FMath::Max(LowY, Y - Step))
		Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));

	X = LowX;
	Y = LowY;
	Fence->SplineComponent->AddSplineWorldPoint(GetVertexPositionWorld(GetVertexIndex(X, Y)) + FVector(0, 0, 30));	
	
	Fence->Regenerate();

	PlacedFences.Add(Fence);
	
	return Fence;
}

TArray<FVertexOverlapInfo> AEcoscapeTerrain::GetVerticiesInSphere(FVector Position, float Radius, bool CheckZ)
{
	SCOPE_CYCLE_COUNTER(STAT_VertsInSphere);

	// DrawDebugSphere(GetWorld(), Position, Radius, 30, FColor::Red);
	
	TArray<FVertexOverlapInfo> Indicies;

	const int RootVertex = GetClosestVertex(Position);
	const int IndexRadius = (Radius / Scale) + 1; // Add 1 so verts that should be inside but aren't due to offsets from the root vertex method are included.

	for (int CurrentRadius = 0; CurrentRadius < IndexRadius; CurrentRadius++)
	{
		// Special case for the root vertex. Just test distance.
		if (CurrentRadius == 0)
		{
			const float Dist = CheckZ ? FVector::Dist(Position, GetVertexPositionWorld(RootVertex)) : FVector::DistXY(Position, GetVertexPositionWorld(RootVertex));
			if (Dist <= Radius)
				Indicies.Add(FVertexOverlapInfo { RootVertex, Dist });
			if (!CheckZ && Dist > Radius)
				return Indicies;
			continue;
		}

		for (int X = -CurrentRadius; X <= CurrentRadius; X++)
		{
			for (int Y = -CurrentRadius; Y <= CurrentRadius; Y++)
			{
				const int Index = RootVertex + X + (Y * (Width + 1));
				
				// Ensure no out of bounds indicies are given
				if (Index < 0 || Index >= Verticies.Num())
					continue;

				const float Dist = CheckZ ? FVector::Dist(Position, GetVertexPositionWorld(Index)) : FVector::DistXY(Position, GetVertexPositionWorld(Index));
				if (Dist <= Radius)
					Indicies.Add(FVertexOverlapInfo { Index, Dist });
			}
		}
	}
	
	return Indicies;
}

void AEcoscapeTerrain::AddVertexColour(int Index, FColor AddedColor, bool Flush)
{
	SCOPE_CYCLE_COUNTER(STAT_AddVertColor);
	
	if (Index < 0 || Index >= Verticies.Num())
	{
		UE_LOG(LogEcoscape, Error, TEXT("AEcoscapeTerrain::AddVertexColour called with out of range index %i (range is 0..%i)"), Index, Verticies.Num());
		return;	
	}

	VertexColors[Index] += AddedColor;
	if (Flush)
		ProceduralMeshComponent->UpdateMeshSection(0, Verticies, Normals, UV0, VertexColors, Tangents);
}

void AEcoscapeTerrain::FlushMesh()
{
	ProceduralMeshComponent->UpdateMeshSection(0, Verticies, Normals, UV0, VertexColors, Tangents);
}

#if WITH_EDITOR
void AEcoscapeTerrain::DrawVerticies()
{
	for (int i = 0; i < Verticies.Num(); i++)
		DrawDebugSphere(GetWorld(), GetVertexPositionWorld(i), 20.0f, 8, FColor::Red, false, 5);
}

void AEcoscapeTerrain::DrawIndicies()
{
	for (int i = 0; i < Verticies.Num(); i++)
		DrawDebugString(GetWorld(), GetVertexPositionWorld(i) + FVector(0, 0, 100), FString::FromInt(i), nullptr, FColor::White, 3);
}
#endif

void AEcoscapeTerrain::Regenerate()
{
	SCOPE_CYCLE_COUNTER(STAT_GenTerrain);
	
	// Re-roll noise seeds	
	for (FTerrainNoiseLayer& Layer : NoiseLayers)
		Layer.Seed = FMath::RandRange(0.0f, 1000000.0f);
	ColorOffsetSeed = FMath::RandRange(0.0f, 1000000.0f);
	
	ResetMeshData();
	for (APlacedItem* Item : PlacedItems)
		Item->Destroy();
	PlacedItems.Empty();
	for (AProceduralFenceMesh* Fence : PlacedFences)
		Fence->Destroy();
	PlacedFences.Empty();
	for (const auto& Detail : DetailActors)
		Detail.Actor->Destroy();
	DetailActors.Empty();
	GenerateVerticies();
	GenerateIndicies();
	GenerateNormals();
	GenerateFence();
	CreateMesh();
	GenerateExteriorDetail();
}
