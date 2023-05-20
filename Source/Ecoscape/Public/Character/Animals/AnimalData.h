// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/DataAsset.h"
#include "AnimalData.generated.h"

class UEnvQuery;
class ABaseAnimal;
/**
 * Data for animals
 */
UCLASS(BlueprintType)
class ECOSCAPE_API UAnimalData : public UDataAsset
{
	GENERATED_BODY()

	UAnimalData();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText SpeciesName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	float BaseHealth = 100;

	/**
	 * Fullness this animal looses per second
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	float HungerRate = 0.005f;

	/**
	 * Thirstiness this animals gains per second
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	float ThirstRate = 0.008f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	TSubclassOf<ABaseAnimal> AnimalClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	UEnvQuery* FoodQuery;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"), Category = "Basic")
	float MoveSpeed = 600;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FVector MeshOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FRotator MeshRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FVector MeshScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	float ColliderHalfHeight = 88;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	float ColliderRadius = 65;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSubclassOf<UAnimInstance> AnimationClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TSubclassOf<AAIController> AI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FVector2D SoundTimeRange = FVector2D(15, 45);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* Sound;
};
