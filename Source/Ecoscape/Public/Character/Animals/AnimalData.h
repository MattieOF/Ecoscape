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

	/**
	 * Fullness this animal looses per second
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HungerRate = 0.005f;

	/**
	 * Thirstiness this animals gains per second
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThirstRate = 0.008f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MoveSpeed = 600;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MeshOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator MeshRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MeshScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ColliderHalfHeight = 88;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ColliderRadius = 65;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimationClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AAIController> AI;
};
