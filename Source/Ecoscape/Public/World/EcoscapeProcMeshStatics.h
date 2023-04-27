// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EcoscapeProcMeshStatics.generated.h"

struct FProcMeshTangent;

/**
 * Useful functions for proc meshes
 */
UCLASS()
class ECOSCAPE_API UEcoscapeProcMeshStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void AddCuboid(TArray<FVector>& Verticies, TArray<int>& Indicies,
	                                                TArray<FVector2D>& UV0, TArray<FVector>& Normals,
	                                                TArray<FProcMeshTangent>& Tangents, FVector Center, FVector Extents,
	                                                bool OverrideUV = false, FVector2D UV = FVector2D(0, 0));
	static void AddCuboid(TArray<FVector>& Verticies, TArray<int>& Indicies, TArray<FVector2D>& UV0,
	                      TArray<FVector>& Normals, TArray<FProcMeshTangent>& Tangents, FVector Start, FVector End,
	                      FVector2D Extents, bool OverrideUV = false, FVector2D UV = FVector2D(0, 0));
};
