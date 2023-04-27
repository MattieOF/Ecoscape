// Copyright 2017-2018 Project Borealis
// with edits by Matt Ware

#pragma once

#include "CoreMinimal.h"
#include "EcoscapeObject.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

#include "Runtime/Launch/Resources/Version.h"

#include "EcoscapeFPPlayerCharacter.generated.h"

class UInteractableComponent;
class USoundCue;
class UPBMoveStepSound;
class UEcoscapeFPPlayerMovement;

inline float SimpleSpline(float Value)
{
	const float ValueSquared = Value * Value;
	return (3.0f * ValueSquared - 2.0f * ValueSquared * Value);
}

UCLASS(config = Game)
class ECOSCAPE_API AEcoscapeFPPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual void ClearJumpInput(float DeltaTime) override;
	virtual void Jump() override;
	virtual void StopJumping() override;
	virtual void OnJumped_Implementation() override;
	virtual bool CanJumpInternal_Implementation() const override;

	virtual void RecalculateBaseEyeHeight() override;

	/* Triggered when player's movement mode has changed */
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;

	float GetLastJumpTime() const
	{
		return LastJumpTime;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContext"))
	static FORCEINLINE AEcoscapeFPPlayerCharacter* GetEcoscapeFPCharacter(UObject* WorldContext, int32 Index = 0)
	{
		return Cast<AEcoscapeFPPlayerCharacter>(UGameplayStatics::GetPlayerPawn(WorldContext, Index));
	}
	
	virtual void Interact();

private:

	/** cached default eye height */
	float DefaultBaseEyeHeight;

	/** when we last jumped */
	float LastJumpTime;

	/** throttle jump boost when going up a ramp, so we don't spam it */
	float LastJumpBoostTime;

	/** maximum time it takes to jump */
	float MaxJumpTime;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Camera")
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate.*/
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Camera")
	float BaseLookUpRate;

	/** Automatic bunnyhopping */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Gameplay")
	bool bAutoBunnyhop;

	/** Move step sounds by physical surface */
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Sounds")
	TMap<TEnumAsByte<EPhysicalSurface>, TSubclassOf<UPBMoveStepSound>> MoveStepSounds;

		/** Minimum speed to play the camera shake for landing */
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Damage")
	float MinLandBounceSpeed;

	/** Don't take damage below this speed - so jumping doesn't damage */
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Damage")
	float MinSpeedForFallDamage;

	// In HL2, the player has the Z component for applying momentum to the capsule capped
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Damage")
	float CapDamageMomentumZ = 0.f;

	/** Pointer to player movement component */
	UPROPERTY()
	UEcoscapeFPPlayerMovement* MovementPtr;

	/** True if we're sprinting*/
	bool bIsSprinting;

	bool bWantsToWalk;

	/** defer the jump stop for a frame (for early jumps) */
	bool bDeferJumpStop;

	float InteractionRange = 200;

	virtual void ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser) override;

	virtual void PossessedBy(AController* NewController) override;

	
protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	AEcoscapeObject* HighlightedObject;
	UPROPERTY()
	UInteractableComponent* CurrentInteractable;
	void HighlightObject(AEcoscapeObject* NewObject);
	
public:
	AEcoscapeFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);

#pragma region Mutators
	UFUNCTION()
	bool IsSprinting() const
	{
		return bIsSprinting;
	}
	UFUNCTION()
	bool DoesWantToWalk() const
	{
		return bWantsToWalk;
	}
	FORCEINLINE TSubclassOf<UPBMoveStepSound>* GetMoveStepSound(TEnumAsByte<EPhysicalSurface> Surface)
	{
		return MoveStepSounds.Find(Surface);
	};
	UFUNCTION(Category = "PB Getters", BlueprintPure) FORCEINLINE float GetBaseTurnRate() const
	{
		return BaseTurnRate;
	};
	UFUNCTION(Category = "PB Setters", BlueprintCallable) void SetBaseTurnRate(float val)
	{
		BaseTurnRate = val;
	};
	UFUNCTION(Category = "PB Getters", BlueprintPure) FORCEINLINE float GetBaseLookUpRate() const
	{
		return BaseLookUpRate;
	};
	UFUNCTION(Category = "PB Setters", BlueprintCallable) void SetBaseLookUpRate(float val)
	{
		BaseLookUpRate = val;
	};
	UFUNCTION(Category = "PB Getters", BlueprintPure) FORCEINLINE bool GetAutoBunnyhop() const
	{
		return bAutoBunnyhop;
	};
	UFUNCTION(Category = "PB Setters", BlueprintCallable) void SetAutoBunnyhop(bool val)
	{
		bAutoBunnyhop = val;
	};
	UFUNCTION(Category = "PB Getters", BlueprintPure) FORCEINLINE UEcoscapeFPPlayerMovement* GetMovementPtr() const
	{
		return MovementPtr;
	};

#pragma endregion Mutators

	float GetDefaultBaseEyeHeight() const { return DefaultBaseEyeHeight; }

	UFUNCTION()
	void ToggleNoClip();

	UFUNCTION(Category = "Player Movement", BlueprintPure)
	float GetMinSpeedForFallDamage() const { return MinSpeedForFallDamage; };

	float GetMinLandBounceSpeed() const { return MinLandBounceSpeed; }

	/** Handles strafing movement, left and right */
	UFUNCTION()
	void Move(FVector Direction, float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param bIsPure If true, rate is not multiplied by turn rate and DeltaTime
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired
	 * turn rate
	 */
	UFUNCTION()
	void Turn(bool bIsPure, float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param bIsPure If true, rate is not multiplied by turn rate and DeltaTime
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired
	 * turn rate
	 */
	UFUNCTION()
	void LookUp(bool bIsPure, float Rate);

	virtual bool CanCrouch() const override;

	virtual void UnPossessed() override;
	
	bool bCurrentlyPossessed = false;
};
