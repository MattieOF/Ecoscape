// copyright lololol

#include "World/FencePlacementPreview.h"

#include "ProceduralMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

AFencePlacementPreview::AFencePlacementPreview()
{
	PrimaryActorTick.bCanEverTick = false;

	auto MatFinder = ConstructorHelpers::FObjectFinder<UMaterialInterface>(TEXT("Material'/Game/Materials/Preview/M_FencePreview.M_FencePreview'"));
	if (MatFinder.Object != nullptr)
		ValidMaterial = MatFinder.Object;
	MatFinder = ConstructorHelpers::FObjectFinder<UMaterialInterface>(TEXT("MaterialInstanceConstant'/Game/Materials/Preview/M_FencePreviewInvalid.M_FencePreviewInvalid'"));
	if (MatFinder.Object != nullptr)
		InvalidMaterial = MatFinder.Object;
	
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	ProceduralMesh->SetMaterial(0, ValidMaterial);
}

void AFencePlacementPreview::DisablePreview()
{
	StartLocation = FVector2D::ZeroVector;
	EndLocation = FVector2D::ZeroVector;
	bEnabled = false;
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
	}
}

void AFencePlacementPreview::RegeneratePreview()
{
	Verticies.Empty();
	Indicies.Empty();
	
	int X = StartLocation.X, Y = StartLocation.Y;
	for (; X < EndLocation.X; X++)
		CreateQuad(Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y)), Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X + 1, Y)));
	for (; Y < EndLocation.Y; Y++)
		CreateQuad(Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y)), Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y + 1)));
	for (; X > StartLocation.X; X--)
		CreateQuad(Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y)), Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X - 1, Y)));
	for (; Y > StartLocation.Y; Y--)
		CreateQuad(Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y)), Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(X, Y - 1)));

	ProceduralMesh->CreateMeshSection(0, Verticies, Indicies, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), false);
}

void AFencePlacementPreview::CheckValid()
{
	bValid = false;
}

void AFencePlacementPreview::CreateQuad(FVector Start, FVector End)
{
	FVector Mid = UKismetMathLibrary::VLerp(Start, End, 0.5);
	FVector Dir = End - Start;
	Dir.Normalize();
	
	
	Verticies.Append({
		
	});
}
