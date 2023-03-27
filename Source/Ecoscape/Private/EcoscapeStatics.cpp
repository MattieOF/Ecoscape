﻿// copyright lololol

#include "EcoscapeStatics.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

FVector2D UEcoscapeStatics::ClampVector2D(FVector2D Vector, FVector2D Min, FVector2D Max)
{
	return FVector2D(FMath::Clamp(Vector.X, Min.X, Max.X), FMath::Clamp(Vector.Y, Min.Y, Max.Y));
}

bool UEcoscapeStatics::GetHitResultAtCursor(const APlayerController* Controller,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, bool bTraceComplex, FHitResult& HitResult,
	const TArray<AActor*>& IgnoredActors)
{
	float MouseX, MouseY;
	if (!Controller->GetMousePosition(MouseX, MouseY))
		return false;

	FVector WorldOrigin;
	FVector WorldDirection;
	if (UGameplayStatics::DeprojectScreenToWorld(Controller, FVector2D(MouseX, MouseY), WorldOrigin, WorldDirection) == true)
	{
		FCollisionObjectQueryParams const ObjParam(ObjectTypes);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ClickableTrace));
		QueryParams.bTraceComplex = bTraceComplex;
		QueryParams.AddIgnoredActors(IgnoredActors);
		return Controller->GetWorld()->LineTraceSingleByObjectType(HitResult, WorldOrigin, WorldOrigin + WorldDirection * Controller->HitResultTraceDistance, ObjParam, QueryParams);
	}

	return false;
}

bool UEcoscapeStatics::GetHitResultAtCursorByChannel(const APlayerController* Controller,
	const TEnumAsByte<ECollisionChannel> CollisionChannel, bool bTraceComplex, FHitResult& HitResult,
	const TArray<AActor*>& IgnoredActors)
{
	float MouseX, MouseY;
	if (!Controller->GetMousePosition(MouseX, MouseY))
		return false;

	FVector WorldOrigin;
	FVector WorldDirection;
	if (UGameplayStatics::DeprojectScreenToWorld(Controller, FVector2D(MouseX, MouseY), WorldOrigin, WorldDirection) == true)
	{
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ClickableTrace));
		QueryParams.bTraceComplex = bTraceComplex;
		QueryParams.AddIgnoredActors(IgnoredActors);
		return Controller->GetWorld()->LineTraceSingleByChannel(HitResult, WorldOrigin, WorldOrigin + WorldDirection * Controller->HitResultTraceDistance, CollisionChannel, QueryParams);
	}

	return false;
}

AActor* UEcoscapeStatics::GetFirstActorWithTag(UObject* WorldContext, FName Tag)
{
	// We do nothing if no tag is provided, rather than giving any actor!
	if (Tag.IsNone())
	{
		return nullptr;
	}
	
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor->ActorHasTag(Tag))
			{
				return Actor;
			}
		}
	}

	return nullptr;
}

void UEcoscapeStatics::TrimTrailingZeros(FString& String)
{
	// Copy-pasted from FString::SanitizeFloat()
	// Trim all trailing zeros (up-to and including the decimal separator) from the fractional part of the number
	int32 TrimIndex = INDEX_NONE;
	int32 DecimalSeparatorIndex = INDEX_NONE;
	for (int32 CharIndex = String.Len() - 1; CharIndex >= 0; --CharIndex)
	{
		const TCHAR Char = String[CharIndex];
		if (Char == TEXT('.'))
		{
			DecimalSeparatorIndex = CharIndex;
			TrimIndex = FMath::Max(TrimIndex, DecimalSeparatorIndex);
			break;
		}
		if (TrimIndex == INDEX_NONE && Char != TEXT('0'))
		{
			TrimIndex = CharIndex + 1;
		}
	}
	check(TrimIndex != INDEX_NONE && DecimalSeparatorIndex != INDEX_NONE);
	String.RemoveAt(TrimIndex, String.Len() - TrimIndex, /*bAllowShrinking*/false);
}

FString UEcoscapeStatics::GetLetterFromNum(int Num)
{
	static FString Alphabet = "abcdefghijklmnopqrstuvwxyz";
	return FString(&Alphabet[Num]);
}

float UEcoscapeStatics::GetZUnderOrigin(AActor* Object)
{
	FVector Origin, Extents;
	Object->GetActorBounds(true, Origin, Extents);
	const FVector ObjectOrigin = Object->GetActorLocation();
	const FVector Difference = Origin - ObjectOrigin;
	return Extents.Z - Difference.Z;
}

void UEcoscapeStatics::SetAllMaterials(UStaticMeshComponent* MeshComponent, UMaterialInterface* Material)
{
	for (int i = 0; i < MeshComponent->GetNumMaterials(); i++)
		MeshComponent->SetMaterial(i, Material);
}