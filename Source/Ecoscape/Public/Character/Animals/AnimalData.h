// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/DataAsset.h"
#include "AnimalData.generated.h"

/**
 * Data for animals
 */
UCLASS(BlueprintType)
class ECOSCAPE_API UAnimalData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpeciesName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseHealth = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AAIController> AI;
};
