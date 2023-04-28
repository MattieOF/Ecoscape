// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EcoscapeObject.h"
#include "GameFramework/Actor.h"
#include "FenceGate.generated.h"

class UProceduralMeshComponent;
class UInteractableComponent;

UCLASS()
class ECOSCAPE_API AFenceGate : public AEcoscapeObject
{
	GENERATED_BODY()

public:
	AFenceGate();

	void Create(FVector Start, FVector End);
	void GenerateNormals();

	void SerialiseGate(FArchive& Ar);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProceduralMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInteractableComponent* Interactable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	UPROPERTY(BlueprintReadWrite)
	FRotator DefaultRotation;
	
private:
	TArray<FVector>   Verticies;
	TArray<FVector>   Normals;
	TArray<FVector2D> UV0;
	TArray<int>       Indicies;
};
