// copyright lololol

#include "World/Fence/FenceGate.h"

#include "ProceduralMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "World/EcoscapeProcMeshStatics.h"
#include "World/InteractableComponent.h"

AFenceGate::AFenceGate()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door Mesh"));
	DoorMesh->ComponentTags.Add("Outline");
	DoorMesh->SetupAttachment(Mesh);
	Interactable = CreateDefaultSubobject<UInteractableComponent>(TEXT("Interactable"));
	Interactable->InteractionName = FText::FromString("Open Gate");
	
	RootComponent = Mesh;

	const auto MatFinder = ConstructorHelpers::FObjectFinder<UMaterialInterface>(TEXT("Material'/Game/Materials/M_Fence.M_Fence'"));
	if (MatFinder.Object != nullptr)
		Material = MatFinder.Object;
}

void AFenceGate::Create(FVector Start, FVector End)
{
	// Update locations
	SetActorLocation(Start);
	End = End - Start;
	Start = FVector::ZeroVector;

	Start.Z -= 18;
	End.Z   -= 18;

	TArray<FProcMeshTangent> TempTangents;
	
	const float DoorWidth = 100.f;
	int VertIndex = Verticies.Num();
	const float Distance = FVector::Distance(Start, End);
	FVector Direction = End - Start;
	Direction.Normalize();

	FVector End1 = Start + (Direction * ((Distance / 2) - (DoorWidth / 2)));
	FVector Start2 = Start + (Direction * ((Distance / 2) + (DoorWidth / 2)));
	
	Verticies.Add(Start - FVector(0, 0, 80));
	Verticies.Add(Start + FVector(0, 0, 160));
	Verticies.Add(End1 - FVector(0, 0, 80));
	Verticies.Add(End1 + FVector(0, 0, 160));
	UV0.Append({ FVector2D(0, 0.0625), FVector2D(0, .5), FVector2D(Distance / 400, 0.0625), FVector2D(Distance / 400, .5) });
	Indicies.Append({ VertIndex + 2, VertIndex + 3, VertIndex + 1, VertIndex + 2, VertIndex + 1, VertIndex });
	
	Verticies.Add(Start2 - FVector(0, 0, 80));
	Verticies.Add(Start2 + FVector(0, 0, 160));
	Verticies.Add(End - FVector(0, 0, 80));
	Verticies.Add(End + FVector(0, 0, 160));
	UV0.Append({ FVector2D(0, 0.0625), FVector2D(0, .5), FVector2D(Distance / 400, 0.0625), FVector2D(Distance / 400, .5) });
	Indicies.Append({ VertIndex + 6, VertIndex + 7, VertIndex + 5, VertIndex + 6, VertIndex + 5, VertIndex + 4 });

	DoorMesh->SetRelativeLocation(FVector(End1.X, End1.Y, FMath::Lerp(Start.Z, End.Z, 0.5f)));
	DefaultRotation = UKismetMathLibrary::Conv_VectorToRotator(Direction);
	DefaultRotation.Yaw += 90;
	DefaultRotation.Pitch = 0;
	DefaultRotation.Roll = 0;
	DoorMesh->SetRelativeRotation(DefaultRotation);

	float PoleZ = End1.Z < Start2.Z ? End1.Z : Start2.Z;
	UEcoscapeProcMeshStatics::AddCuboid(Verticies, Indicies, UV0, Normals, TempTangents, FVector(End1.X, End1.Y, PoleZ) + FVector(0, 0, 90), FVector(10, 10, 160), true, FVector2D(0.1, 0.03));
	UEcoscapeProcMeshStatics::AddCuboid(Verticies, Indicies, UV0, Normals, TempTangents, FVector(Start2.X, Start2.Y, PoleZ) + FVector(0, 0, 90), FVector(10, 10, 160), true, FVector2D(0.1, 0.03));
	
	GenerateNormals();
	
	Mesh->SetMaterial(0, Material);
	Mesh->CreateMeshSection(0, Verticies, Indicies, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void AFenceGate::GenerateNormals()
{
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

void AFenceGate::SerialiseGate(FArchive& Ar)
{
	Ar << Verticies;
	Ar << Indicies;
	Ar << Normals;
	Ar << UV0;

	Mesh->SetMaterial(0, Material);
	Mesh->CreateMeshSection(0, Verticies, Indicies, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);

	FVector Loc = DoorMesh->GetRelativeLocation();
	Ar << Loc;
	FRotator Rot = DoorMesh->GetRelativeRotation();
	Ar << Rot;

	DoorMesh->SetRelativeLocation(Loc);
	DoorMesh->SetRelativeRotation(Rot);
	DefaultRotation = Rot;
}
