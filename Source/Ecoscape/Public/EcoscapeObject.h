// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/OutlineComponent.h"
#include "EcoscapeObject.generated.h"

UCLASS()
class ECOSCAPE_API AEcoscapeObject : public AActor
{
	GENERATED_BODY()

public:
	AEcoscapeObject();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UOutlineComponent* Outline;
};
