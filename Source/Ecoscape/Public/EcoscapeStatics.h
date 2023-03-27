// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EcoscapeStatics.generated.h"

/**
 * Utility functions for Ecoscape
 */
UCLASS()
class ECOSCAPE_API UEcoscapeStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Clamps a vector per-element between a minimum and a maximum vector
	 * @param Vector Vector to be clamped
	 * @param Min Minimum vector
	 * @param Max Maximum vector
	 * @return Vector clamped between Min and Max
	 */
	UFUNCTION(BlueprintCallable, Category = "Math", BlueprintPure)
	static FVector2D ClampVector2D(FVector2D Vector, FVector2D Min, FVector2D Max);

	/**
	 * @brief Utility function similar to GetHitResultUnderCursor, but also includes an IgnoredActors parameter.
	 * @param Controller Player controller to do the check for
	 * @param ObjectTypes Array of of object type queries to check for
	 * @param bTraceComplex If the trace should be against complex collision
	 * @param HitResult Reference to an FHitResult to store data in
	 * @param IgnoredActors Array of actors (AActor pointers) to be ignored by the trace
	 * @return True if the trace hit something, false if not OR if there was an error.
	 */
	UFUNCTION(BlueprintCallable)
	static bool GetHitResultAtCursor(const APlayerController* Controller, const TArray<TEnumAsByte<EObjectTypeQuery> > & ObjectTypes, bool bTraceComplex, FHitResult& HitResult, const TArray<AActor*>& IgnoredActors);

	UFUNCTION(BlueprintCallable)
	static bool GetHitResultAtCursorByChannel(const APlayerController* Controller, const TEnumAsByte<ECollisionChannel> CollisionChannel, bool bTraceComplex, FHitResult& HitResult, const TArray<AActor*>& IgnoredActors);

	/**
	 * @brief Iterates through all actors in the world until it finds one with the provided tag, and returns it. Expensive function; don't call every frame!
	 * @param WorldContext World context object
	 * @param Tag Tag to look for
	 * @return If one was found, the relevant AActor pointer. If not, nullptr.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContext"))
	static AActor* GetFirstActorWithTag(UObject* WorldContext, FName Tag);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static void TrimTrailingZeros(FString& String);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FORCEINLINE int GetUObjectCount() { return GUObjectArray.GetObjectArrayNum(); };

	UFUNCTION(BlueprintCallable)
	static FORCEINLINE void ForceFullPurgeGC() { GEngine->ForceGarbageCollection(true); }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FString GetLetterFromNum(int Num);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetZUnderOrigin(AActor* Object);

	UFUNCTION(BlueprintCallable)
	static void SetAllMaterials(UStaticMeshComponent* MeshComponent, UMaterialInterface* Material);
};
