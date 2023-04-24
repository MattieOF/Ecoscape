// copyright lololol

#include "EcoscapeObject.h"

AEcoscapeObject::AEcoscapeObject()
{
	PrimaryActorTick.bCanEverTick = true;

	Outline = CreateDefaultSubobject<UOutlineComponent>(TEXT("Outline"));
}

void AEcoscapeObject::BeginPlay()
{
	Super::BeginPlay();
	
	Outline->RefreshOutlinedComponents();
}
