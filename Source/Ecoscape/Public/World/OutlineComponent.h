// copyright lolol

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OutlineComponent.generated.h"

UCLASS( ClassGroup=(Ecoscape), meta=(BlueprintSpawnableComponent) )
class ECOSCAPE_API UOutlineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * @brief Sets default values
	 */
	UOutlineComponent();

	/**
	 * @brief Shows the outline for all outlined components
	 */
	UFUNCTION(BlueprintCallable, Category="Outline")
	void ShowOutline();

	/**
	 * @brief Hides the outline for all outlined components
	 */
	UFUNCTION(BlueprintCallable, Category="Outline")
	void HideOutline();

	/**
	 * @brief Looks for all other child components with the tag OutlineTag (variable) and adds them to the internal array.
	 * This function does not show or hide the outline.
	 */
	UFUNCTION(BlueprintCallable, Category="Outline")
	void RefreshOutlinedComponents();

	/**
	 * @brief Tag used when searching for primitive components to be outlined.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outline")
	FName OutlineTag = "Outline";
	
protected:
	/**
	 * @brief Calls RefreshOutlinedComponents()
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief Array of components that should be outlined.
	 */
	UPROPERTY()
	TArray<UPrimitiveComponent*> OutlinedComponents;
	
};
