// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EcoscapePlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "EcoscapePlayerController.generated.h"

UCLASS()
class ECOSCAPE_API AEcoscapePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AEcoscapePlayerController();
	
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnLookUp(float Value);
	void OnTurn(float Value);

	void OnJumpPressed();
	void OnJumpReleased();
	void OnCrouchPressed();
	void OnCrouchReleased();

	UPROPERTY(BlueprintReadOnly)
	AEcoscapePlayerCharacter* EcoscapeCharacter;
};
