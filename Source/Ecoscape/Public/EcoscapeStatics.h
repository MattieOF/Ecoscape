// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EcoscapeStatics.generated.h"

#define ECC_BLOCKS_ITEM_PLACEMENT ECC_GameTraceChannel1
#define ECC_HIGHLIGHTABLE         ECC_GameTraceChannel2
#define ECC_ITEM_PLACEABLE_ON     ECC_GameTraceChannel3

FORCEINLINE FColor operator-(const FColor& LHS, const FColor& RHS)
{
	FColor Result;
	Result.R = static_cast<uint8>(FMath::Clamp(static_cast<int32>(LHS.R) - static_cast<int32>(RHS.R), 0, 255));
	Result.G = static_cast<uint8>(FMath::Clamp(static_cast<int32>(LHS.G) - static_cast<int32>(RHS.G), 0, 255));
	Result.B = static_cast<uint8>(FMath::Clamp(static_cast<int32>(LHS.B) - static_cast<int32>(RHS.B), 0, 255));
	Result.A = static_cast<uint8>(FMath::Clamp(static_cast<int32>(LHS.A) - static_cast<int32>(RHS.A), 0, 255));
	return Result;
}

FORCEINLINE FColor operator*(const FColor& LHS, const float RHS)
{
	FColor Result = LHS;
	Result.R *= RHS;
	Result.G *= RHS;
	Result.B *= RHS;
	return Result;
}

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

	/**
	 * @brief Enables or disables all outlines by enabling or disabling the outline post process volume.
	 * @param WorldContext World context object
	 * @param NewOutlinesEnabled If the outlines should be enabled or not
	 * @param OutlineVolumeTag Tag on the outline post process volume, by default "OutlinePPVolume".
	 * Only the volume should have this tag, as only 1 actor with this tag is considered.
	 */
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContext"))
	static void SetOutlinesEnabled(UObject* WorldContext, bool NewOutlinesEnabled, FName OutlineVolumeTag = "OutlinePPVolume");

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FORCEINLINE float AngleBetweenDirectionsRad(FVector A, FVector B)
	{
		A.Normalize();
		B.Normalize();
		return FMath::Acos(FVector::DotProduct(A, B));
	};
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FORCEINLINE float AngleBetweenDirectionsDeg(const FVector& A, const FVector& B)
	{
		return FMath::RadiansToDegrees(AngleBetweenDirectionsRad(A, B));
	};

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FORCEINLINE FColor AddToColor(FColor Color, const FVector& Value)
	{
		const bool RNeg = Value.X < 0, GNeg = Value.Y < 0, BNeg = Value.Z < 0;
		const FColor R = FColor(FMath::Abs(Value.X), 0, 0),
					 G = FColor(0, FMath::Abs(Value.Y), 0),
					 B = FColor(0, 0, FMath::Abs(Value.Z));
		
		if (RNeg)
			Color = Color - R;
		else
			Color += R;
		if (GNeg)
			Color = Color - G;
		else
			Color += G;
		if (BNeg)
			Color = Color - B;
		else
			Color += B;

		return Color;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FORCEINLINE float MapFloat(const float Value, const float Start1, const float End1, const float Start2, const float End2)
	{
		return Start2 + (End2 - Start2) * ((Value - Start1) / (End1 - Start1));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FORCEINLINE FColor ClampColor(FColor Value, FColor Min, FColor Max)
	{
		FColor Result;
		Result.R = FMath::Clamp(Value.R, Min.R, Max.R);
		Result.G = FMath::Clamp(Value.G, Min.G, Max.G);
		Result.B = FMath::Clamp(Value.B, Min.B, Max.B);
		Result.A = FMath::Clamp(Value.A, Min.A, Max.A);
		return Result;
	}

	static FORCEINLINE FVector GetNonParallelVector(const FVector& Vector)
	{
		// find the smallest component of v (in absolute value)
		double min_abs = abs(Vector.X);
		int min_index = 0;
		if (abs(Vector.Y) < min_abs) {
			min_abs = abs(Vector.Y);
			min_index = 1;
		}
		if (abs(Vector.Z) < min_abs) {
			min_abs = abs(Vector.Z);
			min_index = 2;
		}
    
		// set the smallest component to zero
		FVector NonCollinear = {0, 0, 0};
		if (min_index == 0) {
			NonCollinear.Y = Vector.Z;
			NonCollinear.Z = -Vector.Y;
		} else if (min_index == 1) {
			NonCollinear.X = Vector.Z;
			NonCollinear.Z = -Vector.X;
		} else {
			NonCollinear.X = Vector.Y;
			NonCollinear.Y = -Vector.X;
		}
    
		// normalize the vector
		NonCollinear.Normalize();
    
		return NonCollinear;
	}

	static FString JoinStringArray(const TArray<FString>& Array, const FString& Joiner);
};
