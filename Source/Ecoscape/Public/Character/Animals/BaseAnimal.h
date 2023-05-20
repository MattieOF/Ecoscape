// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "AnimalData.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "BaseAnimal.generated.h"

class ABaseAnimal;
class AEcoscapeTerrain;

USTRUCT()
struct ECOSCAPE_API FHappinessUpdateInfo
{
	GENERATED_BODY()
	
	float PercentageOfHabitatAvailable;
	TArray<int> Reachable;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnimalDies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHappinessUpdated, FHappinessUpdateInfo, Info);

class ECOSCAPE_API FUpdateHappiness : public FRunnable
{
public:
	FUpdateHappiness();
	virtual ~FUpdateHappiness() override;

	virtual bool   Init() override;
	virtual uint32 Run() override;
	virtual void   Exit() override;
	virtual void   Stop() override;

	void EnqueueAnimalForUpdate(ABaseAnimal* Animal, bool bRunIfNot = false);

private:
	
	TQueue<ABaseAnimal*> Animals;
	TArray<ABaseAnimal*> CurrentlyQueued;
	
	FRunnableThread* Thread;
	bool bStopThread = false;
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

	UFUNCTION(BlueprintCallable)
	void SetTerrain(AEcoscapeTerrain* Terrain);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE AEcoscapeTerrain* GetTerrain() { return AssociatedTerrain; }

	UFUNCTION(BlueprintCallable, meta=(WorldContext=World))
	static ABaseAnimal* SpawnAnimal(UObject* World, UAnimalData* Data, AEcoscapeTerrain* Terrain, FVector Position);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateHappiness();
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
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

	UPROPERTY(BlueprintReadWrite)
	bool bIsEating   = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsDrinking = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsSleeping = false;

	UPROPERTY(EditAnywhere)
	bool bDrawNav = false;

	UPROPERTY(BlueprintReadWrite)
	float PercentageOfHabitatAvailable = 0;

	UPROPERTY(BlueprintAssignable)
	FOnHappinessUpdated OnHappinessUpdated;

	UFUNCTION()
	void OnReceiveHappinessUpdated(FHappinessUpdateInfo Info);

	bool bNeedsFreedomUpdate = true;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* Audio;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AEcoscapeTerrain* AssociatedTerrain;
	
private:
	float SoundTimer = 0;
	FRotator TargetRotation;
	static TSharedPtr<FUpdateHappiness> HappinessUpdateRunnable;
	FDelegateHandle TerrainWalkabilityHandle;
};
