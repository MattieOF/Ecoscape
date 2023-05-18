// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "AnimalData.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "NavMesh/RecastNavMesh.h"
#include "BaseAnimal.generated.h"

class AEcoscapeTerrain;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnimalDies);

class FFindAvailableHabitat : public FRunnable
{
public:
	FFindAvailableHabitat();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
};

UCLASS(BlueprintType, Blueprintable)
class ECOSCAPE_API ABaseAnimal : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseAnimal();

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable)
	void SetAnimalData(UAnimalData* Data, bool bRecreateAI = true);

	UFUNCTION(BlueprintCallable, meta=(WorldContext=World))
	static ABaseAnimal* SpawnAnimal(UObject* World, UAnimalData* Data, AEcoscapeTerrain* Terrain, FVector Position);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateHappiness();

	void TerrainFloodFill(TQueue<FVector2D>& Stack, float LX, float RX, float Y, float S);
	
	UPROPERTY(EditAnywhere)
	UAnimalData* AnimalData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GivenName;

	UPROPERTY(BlueprintReadWrite)
	float Health = 100;
	
	/**
	 * The hunger of this animal. 1 is not hungry, 0 is starving.
	 */
	UPROPERTY(BlueprintReadWrite)
	float Hunger = 1;

	/**
	 * The thirst of the animal. 1 is not thirsty, 0 is dying of dehydration.
	 */
	UPROPERTY(BlueprintReadWrite)
	float Thirst = 1;

	UPROPERTY(BlueprintReadWrite)
	float TargetYaw;

	UPROPERTY(BlueprintAssignable)
	FOnAnimalDies OnDeath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AEcoscapeTerrain* AssociatedTerrain;

	UPROPERTY(BlueprintReadWrite)
	bool bIsEating   = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsDrinking = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsSleeping = false;

	UPROPERTY(EditAnywhere)
	bool bDrawNav = false;

private:
	FRotator TargetRotation;

	FRunnableThread* GetHabitatSizeRunnable;
};
