// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "EcoscapeGameModeBase.generated.h"

/**
 * Base game mode for Ecoscape
 */
UCLASS(Blueprintable)
class ECOSCAPE_API AEcoscapeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEcoscapeGameModeBase();

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContext"))
	static FORCEINLINE AEcoscapeGameModeBase* GetEcoscapeBaseGameMode(UObject* WorldContext)
	{
		return Cast<AEcoscapeGameModeBase>(UGameplayStatics::GetGameMode(WorldContext));
	}
};
