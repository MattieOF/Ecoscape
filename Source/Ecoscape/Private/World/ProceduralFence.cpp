// copyright lololol

#include "World/ProceduralFence.h"

#include "EcoscapeStatics.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AProceduralFence::AProceduralFence()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SplineComponent->SetupAttachment(GetRootComponent());
	GetDynamicMeshComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetDynamicMeshComponent()->SetCollisionResponseToChannel(ECC_BLOCKS_ITEM_PLACEMENT, ECR_Block);
}

void AProceduralFence::Generate()
{
	UDynamicMesh* Mesh = GetDynamicMeshComponent()->GetDynamicMesh();
	Mesh->Reset();
	
	FVector LastPosition;
	
	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
	{
		bool ShouldPlace = true;
		if (i == 0 || ShouldPlace)
		{
			FVector Pos = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
			UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(Mesh, FGeometryScriptPrimitiveOptions(), FTransform(Pos), 25, 25, 150, 0, 0, 0);

			if (i != 0)
			{
				FVector MidPoint = UKismetMathLibrary::VLerp(LastPosition, Pos, 0.5f);
				MidPoint.Z += 5;
				float Distance = FVector::Distance(LastPosition, Pos);
				FVector Dir = LastPosition - Pos;
				Dir.Normalize();

				FTransform TF = FTransform(UKismetMathLibrary::Conv_VectorToQuaternion(Dir), MidPoint, FVector::OneVector);
				UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(
					Mesh, FGeometryScriptPrimitiveOptions(),
					TF,
					Distance, 5, 100, 0, 0, 0);
			}
			
			LastPosition = Pos;
		}
	}
}

void AProceduralFence::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Generate();
}
