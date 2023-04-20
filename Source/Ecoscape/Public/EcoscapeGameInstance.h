﻿// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "World/PlaceableItemData.h"
#include "EcoscapeGameInstance.generated.h"

/**
 * Game instance class for Ecoscape. Contains things like a map of placeable items
 */
UCLASS()
class ECOSCAPE_API UEcoscapeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, UPlaceableItemData*> ItemTypes;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContext"))
	static FORCEINLINE UEcoscapeGameInstance* GetEcoscapeGameInstance(UObject* WorldContext)
	{
		return Cast<UEcoscapeGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));
	}
};