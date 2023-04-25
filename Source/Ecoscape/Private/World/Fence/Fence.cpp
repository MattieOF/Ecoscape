// copyright lololol

#include "World/Fence/Fence.h"

#include "Components/SplineMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFence::AFence()
{
	PrimaryActorTick.bCanEverTick = false;

	const auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Meshes/Fence/SM_FencePiece.SM_FencePiece'"));
	if (MeshAsset.Object != nullptr)
		Mesh = MeshAsset.Object;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
}

void AFence::GenerateMesh()
{
	// Remove current spline meshes
	TArray<USceneComponent*> Splines;
	Spline->GetChildrenComponents(false, Splines);
	for (USceneComponent* SplineMesh : Splines)
		SplineMesh->DestroyComponent();

	// Create new splines
	const int SplineLength = FMath::CeilToInt(Spline->GetSplineLength() / Length);
	for (int i = 0; i < SplineLength; i++)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
		SplineMesh->SetupAttachment(Spline);
		SplineMesh->SetStaticMesh(Mesh);
		SplineMesh->SetForwardAxis(ESplineMeshAxis::X);
		SplineMesh->SetSplineUpDir(FVector::UpVector);
		SplineMesh->SetMobility(EComponentMobility::Movable);
		SplineMesh->bSmoothInterpRollScale = false;
		
		const FVector StartLocation = Spline->GetLocationAtDistanceAlongSpline(i * Length, ESplineCoordinateSpace::Local);
		const FVector StartTangent = UKismetMathLibrary::ClampVectorSize(
			Spline->GetTangentAtDistanceAlongSpline(i * Length, ESplineCoordinateSpace::Local), 0, Length);
		const FVector EndLocation = Spline->GetLocationAtDistanceAlongSpline((i + 1) * Length, ESplineCoordinateSpace::Local);
		const FVector EndTangent = UKismetMathLibrary::ClampVectorSize(
			Spline->GetTangentAtDistanceAlongSpline((i + 1) * Length, ESplineCoordinateSpace::Local), 0, Length);
		
		SplineMesh->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent, true);
	}
}

void AFence::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateMesh();
}
