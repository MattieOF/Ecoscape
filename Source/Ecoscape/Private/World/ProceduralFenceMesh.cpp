// copyright lololol

#include "World/ProceduralFenceMesh.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "KismetProceduralMeshLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AProceduralFenceMesh::AProceduralFenceMesh()
{
	PrimaryActorTick.bCanEverTick = false;

	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	ProceduralMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ProceduralMeshComponent->SetCollisionResponseToChannel(ECC_BLOCKS_ITEM_PLACEMENT, ECR_Block);
	ProceduralMeshComponent->SetMaterial(0, Material);
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SplineComponent->SetupAttachment(ProceduralMeshComponent);
	RootComponent = ProceduralMeshComponent;

	const auto MatFinder = ConstructorHelpers::FObjectFinder<UMaterialInterface>(TEXT("Material'/Game/Materials/M_Fence.M_Fence'"));
	if (MatFinder.Object != nullptr)
    	Material = MatFinder.Object;
}

void AProceduralFenceMesh::Regenerate()
{
	Verticies.Empty();
	Indicies.Empty();
	UV0.Empty();
	Normals.Empty();
	Tangents.Empty();
	ProceduralMeshComponent->SetMaterial(0, Material);
	
	FVector LastPosition;
	
	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
	{
		bool ShouldPlace = true;
		if (i == 0 || ShouldPlace)
		{
			FVector Pos = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
			AddCuboid(Pos + FVector(0, 0, 90), FVector(30, 30, 180), true, FVector2D(0.15, 0.03));

			if (i != 0)
			{
				// Gen fence and wire
				AddCuboid(LastPosition + FVector(0, 0, 200), Pos + FVector(0, 0, 200), FVector2D(20, 20), true,
				          FVector2D(0.01, 0.01));

				int VertIndex = Verticies.Num();
				float Distance = FVector::Distance(LastPosition, Pos);
				Verticies.Add(LastPosition - FVector(0, 0, 80));
				Verticies.Add(LastPosition + FVector(0, 0, 160));
				Verticies.Add(Pos - FVector(0, 0, 80));
				Verticies.Add(Pos + FVector(0, 0, 160));
				Indicies.Append({ VertIndex + 2, VertIndex + 3, VertIndex + 1, VertIndex + 2, VertIndex + 1, VertIndex });
				UV0.Append({ FVector2D(0, 0.0625), FVector2D(0, .5), FVector2D(Distance / 200, 0.0625), FVector2D(Distance / 200, .5) });
			}
			
			LastPosition = Pos;
		}
	}

	CalculateNormals();
	// UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Verticies, Indicies, UV0, Normals, Tangents);
	
	ProceduralMeshComponent->CreateMeshSection(0, Verticies, Indicies, Normals, UV0, TArray<FColor>(), Tangents, true);
}

#if WITH_EDITOR
void AProceduralFenceMesh::DrawVerticies()
{
	for (int i = 0; i < Verticies.Num(); i++)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation() + Verticies[i], 20, 6, FColor::Red, false, 2);
		DrawDebugString(GetWorld(), GetActorLocation() + Verticies[i] + FVector(0, 0, 100), FString::FromInt(i),
		                nullptr, FColor::White, 2);
	}
}

void AProceduralFenceMesh::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Regenerate();
}
#endif

void AProceduralFenceMesh::AddCuboid(FVector Center, FVector Extents, bool OverrideUV, FVector2D UV)
{
	// Adapted from UKismetProceduralMeshLibrary::GenerateBoxMesh.

	FVector BoxVerts[8];
	BoxVerts[0] = Center + FVector(-Extents.X, Extents.Y, Extents.Z);
	BoxVerts[1] = Center + FVector(Extents.X, Extents.Y, Extents.Z);
	BoxVerts[2] = Center + FVector(Extents.X, -Extents.Y, Extents.Z);
	BoxVerts[3] = Center + FVector(-Extents.X, -Extents.Y, Extents.Z);
	BoxVerts[4] = Center + FVector(-Extents.X, Extents.Y, -Extents.Z);
	BoxVerts[5] = Center + FVector(Extents.X, Extents.Y, -Extents.Z);
	BoxVerts[6] = Center + FVector(Extents.X, -Extents.Y, -Extents.Z);
	BoxVerts[7] = Center + FVector(-Extents.X, -Extents.Y, -Extents.Z);

	constexpr int32 NumVerts = 24; // 6 faces x 4 verts per face

	const int VertIndex = Verticies.Num(), NormalIndex = Normals.Num(), TangentIndex = Tangents.Num();
	Verticies.AddUninitialized(NumVerts);
	Normals.AddUninitialized(NumVerts);
	Tangents.AddUninitialized(NumVerts);

	Verticies[VertIndex + 0] = BoxVerts[0];
	Verticies[VertIndex + 1] = BoxVerts[1];
	Verticies[VertIndex + 2] = BoxVerts[2];
	Verticies[VertIndex + 3] = BoxVerts[3];
	UKismetProceduralMeshLibrary::UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 0, VertIndex + 1, VertIndex + 2, VertIndex + 3);
	Normals[NormalIndex + 0] = Normals[NormalIndex + 1] = Normals[NormalIndex + 2] = Normals[NormalIndex + 3] = FVector(0, 0, 1);
	Tangents[TangentIndex + 0] = Tangents[TangentIndex + 1] = Tangents[TangentIndex + 2] = Tangents[TangentIndex + 3] = FProcMeshTangent(0.f, -1.f, 0.f);

	Verticies[VertIndex + 4] = BoxVerts[4];
	Verticies[VertIndex + 5] = BoxVerts[0];
	Verticies[VertIndex + 6] = BoxVerts[3];
	Verticies[VertIndex + 7] = BoxVerts[7];
	UKismetProceduralMeshLibrary::UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 4, VertIndex + 5, VertIndex + 6, VertIndex + 7);
	Normals[NormalIndex + 4] = Normals[NormalIndex + 5] = Normals[NormalIndex + 6] = Normals[NormalIndex + 7] = FVector(-1, 0, 0);
	Tangents[TangentIndex + 4] = Tangents[TangentIndex + 5] = Tangents[TangentIndex + 6] = Tangents[TangentIndex + 7] = FProcMeshTangent(0.f, -1.f, 0.f);

	Verticies[VertIndex + 8] = BoxVerts[5];
	Verticies[VertIndex + 9] = BoxVerts[1];
	Verticies[VertIndex + 10] = BoxVerts[0];
	Verticies[VertIndex + 11] = BoxVerts[4];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 8, VertIndex + 9, VertIndex + 10, VertIndex + 11);
	Normals[NormalIndex + 8] = Normals[NormalIndex + 9] = Normals[NormalIndex + 10] = Normals[NormalIndex + 11] = FVector(0, 1, 0);
	Tangents[TangentIndex + 8] = Tangents[TangentIndex + 9] = Tangents[TangentIndex + 10] = Tangents[TangentIndex + 11] = FProcMeshTangent(-1.f, 0.f, 0.f);

	Verticies[VertIndex + 12] = BoxVerts[6];
	Verticies[VertIndex + 13] = BoxVerts[2];
	Verticies[VertIndex + 14] = BoxVerts[1];
	Verticies[VertIndex + 15] = BoxVerts[5];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 12, VertIndex + 13, VertIndex + 14, VertIndex + 15);
	Normals[NormalIndex + 12] = Normals[NormalIndex + 13] = Normals[NormalIndex + 14] = Normals[NormalIndex + 15] = FVector(1, 0, 0);
	Tangents[TangentIndex + 12] = Tangents[TangentIndex + 13] = Tangents[TangentIndex + 14] = Tangents[TangentIndex + 15] = FProcMeshTangent(0.f, 1.f, 0.f);

	Verticies[VertIndex + 16] = BoxVerts[7];
	Verticies[VertIndex + 17] = BoxVerts[3];
	Verticies[VertIndex + 18] = BoxVerts[2];
	Verticies[VertIndex + 19] = BoxVerts[6];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 16, VertIndex + 17, VertIndex + 18, VertIndex + 19);
	Normals[NormalIndex + 16] = Normals[NormalIndex + 17] = Normals[NormalIndex + 18] = Normals[NormalIndex + 19] = FVector(0, -1, 0);
	Tangents[TangentIndex + 16] = Tangents[TangentIndex + 17] = Tangents[TangentIndex + 18] = Tangents[TangentIndex + 19] = FProcMeshTangent(1.f, 0.f, 0.f);

	Verticies[VertIndex + 20] = BoxVerts[7];
	Verticies[VertIndex + 21] = BoxVerts[6];
	Verticies[VertIndex + 22] = BoxVerts[5];
	Verticies[VertIndex + 23] = BoxVerts[4];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 20, VertIndex + 21, VertIndex + 22, VertIndex + 23);
	Normals[NormalIndex + 20] = Normals[NormalIndex + 21] = Normals[NormalIndex + 22] = Normals[NormalIndex + 23] = FVector(0, 0, -1);
	Tangents[TangentIndex + 20] = Tangents[TangentIndex + 21] = Tangents[TangentIndex + 22] = Tangents[TangentIndex + 23] = FProcMeshTangent(0.f, 1.f, 0.f);

	// UVs
	const int UVIndex = UV0.Num();
	UV0.AddUninitialized(NumVerts);
	if (OverrideUV)
	{
		UV0[UVIndex] = UV0[UVIndex + 4] = UV0[UVIndex + 8] = UV0[UVIndex + 12] = UV0[UVIndex + 16] = UV0[UVIndex + 20] = UV;
		UV0[UVIndex + 1] = UV0[UVIndex + 5] = UV0[UVIndex + 9] = UV0[UVIndex + 13] = UV0[UVIndex + 17] = UV0[UVIndex + 21] = UV;
		UV0[UVIndex + 2] = UV0[UVIndex + 6] = UV0[UVIndex + 10] = UV0[UVIndex + 14] = UV0[UVIndex + 18] = UV0[UVIndex + 22] = UV;
		UV0[UVIndex + 3] = UV0[UVIndex + 7] = UV0[UVIndex + 11] = UV0[UVIndex + 15] = UV0[UVIndex + 19] = UV0[UVIndex + 23] = UV;
	} else
	{
		UV0[UVIndex] = UV0[UVIndex + 4] = UV0[UVIndex + 8] = UV0[UVIndex + 12] = UV0[UVIndex + 16] = UV0[UVIndex + 20] = FVector2D(0.f, 0.f);
		UV0[UVIndex + 1] = UV0[UVIndex + 5] = UV0[UVIndex + 9] = UV0[UVIndex + 13] = UV0[UVIndex + 17] = UV0[UVIndex + 21] = FVector2D(0.f, 1.f);
		UV0[UVIndex + 2] = UV0[UVIndex + 6] = UV0[UVIndex + 10] = UV0[UVIndex + 14] = UV0[UVIndex + 18] = UV0[UVIndex + 22] = FVector2D(1.f, 1.f);
		UV0[UVIndex + 3] = UV0[UVIndex + 7] = UV0[UVIndex + 11] = UV0[UVIndex + 15] = UV0[UVIndex + 19] = UV0[UVIndex + 23] = FVector2D(1.f, 0.f);
	}
}

void AProceduralFenceMesh::AddCuboid(FVector Start, FVector End, FVector2D Extents, bool OverrideUV, FVector2D UV)
{
	FVector BoxVerts[8];
	FVector Dir = End - Start;
	Dir.Normalize();

	// Get first quad
	FVector V = UEcoscapeStatics::GetNonParallelVector(Dir);
	FVector U = FVector::CrossProduct(V, Dir);
	FVector W = FVector::CrossProduct(Dir, U);

	BoxVerts[0] = Start + Extents.X * U + Extents.Y * W;
	BoxVerts[1] = Start + Extents.X * U - Extents.Y * W;
	BoxVerts[2] = Start - Extents.X * U - Extents.Y * W;
	BoxVerts[3] = Start - Extents.X * U + Extents.Y * W;

	// And the second quad
	Dir = Start - End;
	Dir.Normalize();
	V = UEcoscapeStatics::GetNonParallelVector(Dir);
	U = FVector::CrossProduct(V, Dir);
	W = FVector::CrossProduct(Dir, U);

	BoxVerts[4] = End + Extents.X * U + Extents.Y * W;
	BoxVerts[5] = End + Extents.X * U - Extents.Y * W;
	BoxVerts[6] = End - Extents.X * U - Extents.Y * W;
	BoxVerts[7] = End - Extents.X * U + Extents.Y * W;
	
	constexpr int32 NumVerts = 24; // 6 faces x 4 verts per face

	// for (int i = 0; i < 8; i++)
	// {
	// 	DrawDebugSphere(GetWorld(), GetActorLocation() + BoxVerts[i], 20, 6, FColor::Red, false, 20);
	// 	DrawDebugString(GetWorld(), GetActorLocation() + BoxVerts[i] + FVector(0, 0, 100), FString::FromInt(i),
	// 					nullptr, FColor::White, 20);
	// }
	
	const int VertIndex = Verticies.Num(), NormalIndex = Normals.Num(), TangentIndex = Tangents.Num();
	Verticies.AddZeroed(NumVerts);
	Normals.AddZeroed(NumVerts);
	Tangents.AddZeroed(NumVerts);

	Verticies[VertIndex + 3] = BoxVerts[0];
	Verticies[VertIndex + 2] = BoxVerts[1];
	Verticies[VertIndex + 1] = BoxVerts[2];
	Verticies[VertIndex] = BoxVerts[3];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex, VertIndex + 1, VertIndex + 2, VertIndex + 3);
	
	Verticies[VertIndex + 7] = BoxVerts[4];
	Verticies[VertIndex + 6] = BoxVerts[5];
	Verticies[VertIndex + 5] = BoxVerts[6];
	Verticies[VertIndex + 4] = BoxVerts[7];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 4, VertIndex + 5, VertIndex + 6, VertIndex + 7);

	Verticies[VertIndex + 8] = BoxVerts[3];
	Verticies[VertIndex + 9] = BoxVerts[0];
	Verticies[VertIndex + 10] = BoxVerts[5];
	Verticies[VertIndex + 11] = BoxVerts[6];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 8, VertIndex + 9, VertIndex + 10, VertIndex + 11);
	
	Verticies[VertIndex + 12] = BoxVerts[7];
	Verticies[VertIndex + 13] = BoxVerts[4];
	Verticies[VertIndex + 14] = BoxVerts[1];
	Verticies[VertIndex + 15] = BoxVerts[2];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 12, VertIndex + 13, VertIndex + 14, VertIndex + 15);
	
	Verticies[VertIndex + 16] = BoxVerts[1];
	Verticies[VertIndex + 17] = BoxVerts[4];
	Verticies[VertIndex + 18] = BoxVerts[5];
	Verticies[VertIndex + 19] = BoxVerts[0];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 16, VertIndex + 17, VertIndex + 18, VertIndex + 19);
	
	Verticies[VertIndex + 20] = BoxVerts[6];
	Verticies[VertIndex + 21] = BoxVerts[7];
	Verticies[VertIndex + 22] = BoxVerts[2];
	Verticies[VertIndex + 23] = BoxVerts[3];
	UKismetProceduralMeshLibrary::ConvertQuadToTriangles(Indicies, VertIndex + 20, VertIndex + 21, VertIndex + 22, VertIndex + 23);
	
	const int UVIndex = UV0.Num();
	UV0.AddUninitialized(NumVerts);
	UV0[UVIndex] = UV0[UVIndex + 4] = UV0[UVIndex + 8] = UV0[UVIndex + 12] = UV0[UVIndex + 16] = UV0[UVIndex + 20] = OverrideUV ? UV : FVector2D(0.f, 0.f);
	UV0[UVIndex + 1] = UV0[UVIndex + 5] = UV0[UVIndex + 9] = UV0[UVIndex + 13] = UV0[UVIndex + 17] = UV0[UVIndex + 21] = OverrideUV ? UV : FVector2D(0.f, 1.f);
	UV0[UVIndex + 2] = UV0[UVIndex + 6] = UV0[UVIndex + 10] = UV0[UVIndex + 14] = UV0[UVIndex + 18] = UV0[UVIndex + 22] = OverrideUV ? UV : FVector2D(1.f, 1.f);
	UV0[UVIndex + 3] = UV0[UVIndex + 7] = UV0[UVIndex + 11] = UV0[UVIndex + 15] = UV0[UVIndex + 19] = UV0[UVIndex + 23] = OverrideUV ? UV : FVector2D(1.f, 0.f);
}

void AProceduralFenceMesh::CalculateNormals()
{
	// BEEFY SOLUTION
	Normals.Init(FVector::ZeroVector, Verticies.Num());

	for (int i = 0; i < Indicies.Num(); i += 3)
	{
		const FVector P = FVector::CrossProduct(Verticies[Indicies[i + 1]] - Verticies[Indicies[i]], Verticies[Indicies[i + 2]] - Verticies[Indicies[i]]);
		Normals[Indicies[i]] += P;
		Normals[Indicies[i + 1]] += P;
		Normals[Indicies[i + 2]] += P;
	}

	for (int i = 0; i < Normals.Num(); i++)
	{
		Normals[i].Normalize();
		Normals[i] = -Normals[i];
	}
}
