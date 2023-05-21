// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "PlacedItem.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "EcoscapeTerrain.generated.h"

class AStaticMeshActor;
class AFenceGate;
class AProceduralFenceMesh;

USTRUCT()
struct FTerrainStartingObjectType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UPlaceableItemData*  Item = nullptr;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ActorType = nullptr;
};

DECLARE_MULTICAST_DELEGATE(FTerrainWalkabilityUpdated);

FORCEINLINE FArchive& operator<<(FArchive& LHS, FProcMeshTangent& RHS)
{
	LHS << RHS.TangentX;
	LHS << RHS.bFlipTangentY;
	return LHS;
}

UENUM(BlueprintType)
enum ENoiseType
{
	ENTPerlin    UMETA(DisplayName = "Perlin"),
	ENTSimplex   UMETA(DisplayName = "Simplex")
};

UENUM()
enum EFenceImpl
{
	EFISplineMesh  UMETA(DisplayName = "Spline Mesh"),
	EFIGeoScript   UMETA(DisplayName = "Geometry Script"),
	EFIProc        UMETA(DisplayName = "Procedural")
};

USTRUCT(BlueprintType)
struct FVertexOverlapInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Index;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Distance;
};

USTRUCT(BlueprintType)
struct FTerrainNoiseLayer
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ENoiseType> NoiseType = ENTPerlin;
	
	UPROPERTY(EditAnywhere)
	float CoordinateScale = .1f;

	UPROPERTY(EditAnywhere)
	float HeightScale = 100.0f;
	
	UPROPERTY(EditAnywhere)
	float Seed = 0;

	UPROPERTY(EditAnywhere)
	float Offset = .1f;
};

USTRUCT()
struct FTerrainDetailActor
{
	GENERATED_BODY()

	UPROPERTY()
	FString ItemName;
	UPROPERTY()
	FVector Pos;

	UPROPERTY()
	AStaticMeshActor* Actor = nullptr;
};

FORCEINLINE FArchive& operator<<(FArchive& LHS, FTerrainDetailActor& RHS)
{
	LHS << RHS.ItemName;
	LHS << RHS.Pos;
	
	return LHS;
}

USTRUCT(BlueprintType)
struct FDrinkLocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector Location;
	UPROPERTY(BlueprintReadWrite)
	FVector DrinkerOrientation;
};

FORCEINLINE FArchive& operator<<(FArchive& LHS, FDrinkLocation& RHS)
{
	LHS << RHS.Location;
	LHS << RHS.DrinkerOrientation;
	
	return LHS;
}

// USTRUCT(BlueprintType)
// struct FFenceInfo
// {
// 	GENERATED_BODY()
//
// 	FVector2D Start, End;
// 	int Step = 2;
// };

UCLASS()
class ECOSCAPE_API AEcoscapeTerrain : public AActor
{
	GENERATED_BODY()

public:
	AEcoscapeTerrain();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString TerrainName = "Terrain";

	UPROPERTY()
	TArray<APlacedItem*> PlacedItems;

	UPROPERTY()
	TArray<AProceduralFenceMesh*> PlacedFences;

	UPROPERTY(EditAnywhere)
	TArray<UPlaceableItemData*> ExteriorItems;

	UPROPERTY(EditAnywhere)
	int ExteriorItemCount = 600;

	UPROPERTY(EditAnywhere)
	FLinearColor FloorColour;
	UPROPERTY(EditAnywhere)
	FLinearColor ExteriorColour;

	UPROPERTY(EditAnywhere)
	bool bDoRotationForExteriorItems = true;

	TArray<FTerrainDetailActor> DetailActors; 

	UPROPERTY(EditAnywhere)
	int Width = 50;
	UPROPERTY(EditAnywhere)
	int Height = 50;

	// UPROPERTY()
	// TArray<FFenceInfo> PlacedFenceInfo;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EFenceImpl> FenceImplementation;
#endif
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProceduralFenceMesh> FenceClass;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE int GetGIIndex() { return GIIndex; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE FVector GetVertexPositionLocal(int Index) { return Verticies[Index]; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE FVector GetVertexPositionWorld(int Index) { return GetActorLocation() + GetVertexPositionLocal(Index); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetScale() const { return Scale; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetHighestHeight() const { return HighestHeight; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetLowestHeight() const { return LowestHeight; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetCenterPosition();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetXYBounds(FVector2D& Min, FVector2D& Max);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetClosestVertex(FVector Position);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE int GetVertexIndex(int X, int Y)
	{
		return (X * (Width + 1)) + Y;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE FVector2D GetVertexXY(int Index)
	{
		return FVector2D(FMath::Floor(Index / (Width + 1)), Index % (Height + 1));
	}

	FORCEINLINE float GetWaterHeight() const
	{
		return Water ? Water->GetActorLocation().Z : -100000;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPositionWithinPlayableSpace(FVector Position);

	UFUNCTION(BlueprintCallable)
	AProceduralFenceMesh* CreateFence(FVector2D Start, FVector2D End, int Step = 2);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FVertexOverlapInfo> GetVerticiesInSphere(FVector Position, float Radius, bool CheckZ = false);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector FindSpawnPoint();

	UFUNCTION(BlueprintCallable)
	void AddVertexColour(int Index, FColor AddedColor, bool Flush = true);

	UFUNCTION(BlueprintCallable)
	void CalculateVertColour(int Index, bool Flush = false);

	UFUNCTION(BlueprintCallable)
	void FlushMesh();
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void Regenerate();
	
	UPROPERTY(EditAnywhere)
	int ExteriorTileCount = 30;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetPlayablePoints(TArray<FVector>& OutPoints);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetRandomDrinkLocation(FDrinkLocation& OutDrinkLocation);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetClosestDrinkLocation(FVector Origin, FDrinkLocation& OutDrinkLocation);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE int GetWalkableVertCount() { return (Width - (ExteriorTileCount * 2 + 4)) * (Height - (ExteriorTileCount * 2 + 4)) - WaterHeight * Heights.Num(); }
	// FORCEINLINE int GetWalkableVertCount() { int Num = 0; for (bool b : Walkable) if (b) Num++; return Num; }

	UFUNCTION(BlueprintCallable)
	bool IsVertWalkable(int Index, bool bDiscountWet = true);
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FDrinkLocation> DrinkLocations;

	UPROPERTY(BlueprintReadOnly)
	TArray<bool> Walkable;
	
	TArray<int> GetVertexesWithinBounds(FVector Origin, FVector Extents, bool bIgnoreZ);
	
	UFUNCTION(BlueprintCallable)
	void OnItemPlaced(APlacedItem* NewItem);

	UFUNCTION(BlueprintCallable)
	TArray<int> GetFenceVerticies(FVector2D Start, FVector2D End);
	
	UFUNCTION(BlueprintCallable)
	void OnFencePlaced(FVector2D Start, FVector2D End);
	
	FTerrainWalkabilityUpdated WalkabilityUpdated;
	
	// -----------------------
	// Serialisation functions
	// -----------------------
	void SerialiseTerrain(FArchive& Archive);
	UFUNCTION(BlueprintCallable, CallInEditor)
	bool SerialiseTerrainToFile(FString Filename = "Terrain.esl");
	UFUNCTION(BlueprintCallable, CallInEditor)
	bool DeserialiseTerrainFromFile(FString Filename = "Terrain.esl");
#if WITH_EDITOR
	// These two functions are for buttons in the inspector to test serialisation
	UFUNCTION(BlueprintCallable, CallInEditor)
	void SerialiseTerrainToTestFile() { SerialiseTerrainToFile(); }
	UFUNCTION(BlueprintCallable, CallInEditor)
	void DeserialiseTerrainFromTestFile() { DeserialiseTerrainFromFile(); }
#endif

	void CalculateDiversity();
	
	UPROPERTY(BlueprintReadOnly)
	float Diversity = 0;

	UPROPERTY(BlueprintReadOnly)
	FText DiversityMessage = FText::FromString("");

	UPROPERTY(EditAnywhere)
	TArray<FTerrainStartingObjectType> StartingItems;

	UPROPERTY(EditAnywhere)
	int StartingItemCount = 0;
	
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void ResetMeshData();
	void GenerateVerticies();
	void GenerateIndicies();
	void GenerateNormals();
	void GenerateFence();
	void CreateNavVolume();
	void GenerateDrinkLocations();
	void CreateMesh() const;
	void GenerateExteriorDetail();
	void GenerateStartingItems();
	void InitWalkable();

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor)
	void DrawVerticies();
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void DrawIndicies();
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void DrawDrinkLocations();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void DrawWalkability();
#endif
	
	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(EditAnywhere)
	float ExteriorHeightOffset = 1500;
	
	/**
	 * @brief Distance in units between each vertex
	 */
	UPROPERTY(EditAnywhere)
	float Scale = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	bool bEnableWater = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	TSubclassOf<AActor> WaterClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float WaterHeight = 0.5f;

	/**
	 * @brief UV scale
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin=0.0001))
	float UVScale = 1;

	UPROPERTY(EditAnywhere)
	FVector2D ColorOffsetRange = FVector2D(-30, 15);
	
	UPROPERTY(EditAnywhere)
	TArray<FTerrainNoiseLayer> NoiseLayers;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	// Mesh data
	UPROPERTY()
	TArray<FVector> Verticies;
	UPROPERTY()
	TArray<FColor> VertexColors;
	UPROPERTY()
	TArray<int> Triangles;
	UPROPERTY()
	TArray<FVector2D> UV0;
	UPROPERTY()
	TArray<FVector> Normals;
	UPROPERTY()
	TArray<FProcMeshTangent> Tangents;
	UPROPERTY()
	TArray<FVector> ColorOffsets;

	UPROPERTY()
	AProceduralFenceMesh* FenceMesh = nullptr;
	UPROPERTY()
	AActor* Water = nullptr;
	UPROPERTY()
	AActor* NavMeshVolume = nullptr;

	// Generation stuff
	UPROPERTY(BlueprintReadOnly)
	float ColorOffsetSeed = 0;
	UPROPERTY(BlueprintReadOnly)
	float LowestHeight = 100000; // Set to a large number so it does get set
	UPROPERTY(BlueprintReadOnly)
	float HighestHeight = 0;
	UPROPERTY(BlueprintReadOnly)
	float AverageHeight = 0;
	UPROPERTY(BlueprintReadOnly)
	TArray<float> Heights;

	int GIIndex = 0;
};
