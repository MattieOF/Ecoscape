// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EcoscapeFPPlayerCharacter.h"
#include "EcoscapeTDCharacter.h"
#include "GameFramework/PlayerController.h"
#include "EcoscapePlayerController.generated.h"

UENUM(BlueprintType)
enum EEcoscapePlayerView
{
	EPSFirstPerson   UMETA(DisplayName = "First Person"),
	EPSTopDown       UMETA(DisplayName = "Top Down")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerViewChanged, EEcoscapePlayerView, NewView);

UCLASS()
class ECOSCAPE_API AEcoscapePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AEcoscapePlayerController();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContext"))
	static FORCEINLINE AEcoscapePlayerController* GetEcoscapePlayerController(UObject* WorldContext, int32 Index = 0)
	{
		return Cast<AEcoscapePlayerController>(UGameplayStatics::GetPlayerController(WorldContext, Index));
	}
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsModifierHeld() { return bModifierPressed; }
	
	UFUNCTION(BlueprintCallable)
	void SetView(EEcoscapePlayerView NewView, bool bInstant = false, float BlendTime = 0.5f);

	UFUNCTION(BlueprintCallable)
	void SetMouseEnabled(bool NewState);

	UFUNCTION(BlueprintCallable)
	void CenterMouse();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AEcoscapeTDCharacter> TopDownCharacterClass = AEcoscapeTDCharacter::StaticClass();

	UPROPERTY(BlueprintAssignable)
	FOnPlayerViewChanged OnPlayerViewChanged;
	
protected:
	virtual void BeginPlay() override;

	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnLookUp(float Value);
	void OnTurn(float Value);
	void OnScroll(float Value);

	void OnJumpPressed();
	void OnJumpReleased();
	void OnCrouchPressed();
	void OnCrouchReleased();

	void OnModifierPressed();
	void OnModifierReleased();

	void OnSwitchView();
	void OnUseTool();
	void OnUseAltTool();
	void OnResetTool();

	UFUNCTION(BlueprintImplementableEvent)
	void OnEcoscapePlayerViewChanged(EEcoscapePlayerView NewState, APawn* NewPawn, float Time = 0.5f);

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EEcoscapePlayerView> CurrentView;

	UPROPERTY(BlueprintReadOnly)
	APawn* CurrentPawn;
	
	UPROPERTY(BlueprintReadOnly)
	AEcoscapeFPPlayerCharacter* FPCharacter;

	UPROPERTY(BlueprintReadOnly)
	AEcoscapeTDCharacter* TDCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SwitchViewCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintSpeedModifier = 2;
	
private:
	float CurrentSwitchViewCooldown = 0;

	bool bModifierPressed = false;
};
