// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetFactories.generated.h"

UCLASS()
class UPlaceableItemFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UPlaceableItemFactory();
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

UCLASS()
class UAnimalDataFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UAnimalDataFactory();
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
