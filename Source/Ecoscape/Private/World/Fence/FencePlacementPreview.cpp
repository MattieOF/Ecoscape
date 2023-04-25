// copyright lololol

#include "World/Fence/FencePlacementPreview.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "ProceduralMeshComponent.h"
#include "World/EcoscapeTerrain.h"

// Utility macro for CheckValid()
#define INVALID_IF(Cond) if (Cond)\
	{\
	bValid = false;\
	ProceduralMesh->SetMaterial(0, InvalidMaterial);\
	return;\
	}\

AFencePlacementPreview::AFencePlacementPreview()
{
	PrimaryActorTick.bCanEverTick = false;

	const auto ValidMatFinder = ConstructorHelpers::FObjectFinder<UMaterialInterface>(TEXT("Material'/Game/Materials/Preview/M_FencePreview.M_FencePreview'"));
	if (ValidMatFinder.Object != nullptr)
		ValidMaterial = ValidMatFinder.Object;
	const auto InvalidMatFinder = ConstructorHelpers::FObjectFinder<UMaterialInterface>(TEXT("MaterialInstanceConstant'/Game/Materials/Preview/M_FencePreviewInvalid.M_FencePreviewInvalid'"));
	if (InvalidMatFinder.Object != nullptr)
		InvalidMaterial = InvalidMatFinder.Object;
	
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	ProceduralMesh->SetMaterial(0, ValidMaterial);
}

void AFencePlacementPreview::DisablePreview()
{
	// Update state
	StartLocation = FVector2D::ZeroVector;
	EndLocation = FVector2D::ZeroVector;
	bEnabled = false;
	bValid = false;

	// Update mesh
	Verticies.Empty();
	Indicies.Empty();
	ProceduralMesh->ClearMeshSection(0);
}

void AFencePlacementPreview::StartPreview(FVector2D StartPos)
{
	StartLocation = StartPos;
	EndLocation = StartPos;
	bEnabled = true;
	bValid = false;
}

void AFencePlacementPreview::UpdatePreview(FVector2D EndPos)
{
	if (EndLocation != EndPos)
	{
		EndLocation = EndPos;
		RegeneratePreview();
		CheckValid();
	}
}

void AFencePlacementPreview::RegeneratePreview()
{
	Verticies.Empty();
	Indicies.Empty();

	const int LowX = StartLocation.X < EndLocation.X ? StartLocation.X : EndLocation.X;
	const int HighX = StartLocation.X > EndLocation.X ? StartLocation.X : EndLocation.X;
	const int LowY = StartLocation.Y < EndLocation.Y ? StartLocation.Y : EndLocation.Y;
	const int HighY = StartLocation.Y > EndLocation.Y ? StartLocation.Y : EndLocation.Y;
	int X = LowX, Y = LowY;
	for (; X < HighX; X++)
		CreateQuad(Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y)), Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X + 1, Y)));
	for (; Y < HighY; Y++)
		CreateQuad(Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y)), Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y + 1)));
	for (; X > LowX; X--)
		CreateQuad(Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y)), Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X - 1, Y)));
	for (; Y > LowY; Y--)
		CreateQuad(Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y)), Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y - 1)));

	ProceduralMesh->CreateMeshSection(0, Verticies, Indicies, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), false);
}

void AFencePlacementPreview::CheckValid()
{
	bValid = true;

	const FVector StartLoc = Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(StartLocation.X, StartLocation.Y));
	const FVector EndLoc = Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(EndLocation.X, EndLocation.Y));
	const float LowX = StartLoc.X < EndLoc.X ? StartLoc.X : EndLoc.X;
	const float HighX = StartLoc.X > EndLoc.X ? StartLoc.X : EndLoc.X;
	const float LowY = StartLoc.Y < EndLoc.Y ? StartLoc.Y : EndLoc.Y;
	const float HighY = StartLoc.Y > EndLoc.Y ? StartLoc.Y : EndLoc.Y;

	FCollisionShape CollisionShape;
	
	INVALID_IF(StartLocation.X == EndLocation.X || StartLocation.Y == EndLocation.Y);
	CollisionShape.SetBox(FVector3f((HighX - LowX) / 2, 30, 5000));
	INVALID_IF(GetWorld()->OverlapBlockingTestByChannel(FVector(LowX + (HighX - LowX) / 2, LowY, 0), FQuat::Identity, ECC_BLOCKS_ITEM_PLACEMENT, CollisionShape));
	INVALID_IF(GetWorld()->OverlapBlockingTestByChannel(FVector(LowX + (HighX - LowX) / 2, HighY, 0), FQuat::Identity, ECC_BLOCKS_ITEM_PLACEMENT, CollisionShape));
	CollisionShape.SetBox(FVector3f(30, (HighY - LowY) / 2, 5000));
	INVALID_IF(GetWorld()->OverlapBlockingTestByChannel(FVector(LowX, LowY + (HighY - LowY) / 2, 0), FQuat::Identity, ECC_BLOCKS_ITEM_PLACEMENT, CollisionShape));
	INVALID_IF(GetWorld()->OverlapBlockingTestByChannel(FVector(HighX, LowY + (HighY - LowY) / 2, 0), FQuat::Identity, ECC_BLOCKS_ITEM_PLACEMENT, CollisionShape));

	ProceduralMesh->SetMaterial(0, ValidMaterial);
}

void AFencePlacementPreview::CreateFence() const
{
	if (!Terrain)
	{
		UE_LOG(LogEcoscape, Error, TEXT("Attempting to create fence in fence preview without assossiated terrain!"));
		return;
	}

	if (!bValid)
		UE_LOG(LogEcoscape, Warning, TEXT("Creating invalid fence in AFencePlacementPreview"));

	Terrain->CreateFence(StartLocation, EndLocation);
}

void AFencePlacementPreview::CreateQuad(FVector Start, FVector End, float Thickness, float ZOffset)
{
	Start.Z += ZOffset;
	End.Z   += ZOffset;
	
	const FVector Direction = End - Start;
	FVector Perpendicular = Direction.Cross(FVector(0, 0, 1));
	if (Perpendicular.Length() < 1e-5)
		Perpendicular = Direction.Cross(FVector(0, 1, 0));
	const FVector UnitPerpendicular = Perpendicular / Perpendicular.Length();

	const float HalfThickness = Thickness / 2;

	int VertIndex = Verticies.Num();
	Verticies.Append({
		Start + HalfThickness * UnitPerpendicular,
		Start - HalfThickness * UnitPerpendicular,
		End   - HalfThickness * UnitPerpendicular,
		End   + HalfThickness * UnitPerpendicular,
	});
	Indicies.Append({
		VertIndex, VertIndex + 1, VertIndex + 2,
		VertIndex + 2, VertIndex + 3, VertIndex
	});
}
