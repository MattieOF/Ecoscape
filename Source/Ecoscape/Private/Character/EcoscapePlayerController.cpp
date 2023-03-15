// copyright lololol

// Disable this warning as Rider thinks the input functions can be const,
// but the binding functions don't allow const functions
// ReSharper disable CppMemberFunctionMayBeConst

#include "Character/EcoscapePlayerController.h"

#include "Kismet/GameplayStatics.h"

AEcoscapePlayerController::AEcoscapePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEcoscapePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Get character
	EcoscapeCharacter = Cast<AEcoscapePlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	// Setup input
	InputComponent->BindAxis("MoveForward", this, &AEcoscapePlayerController::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &AEcoscapePlayerController::OnMoveRight);
	InputComponent->BindAxis("LookUp", this, &AEcoscapePlayerController::OnLookUp);
	InputComponent->BindAxis("Turn", this, &AEcoscapePlayerController::OnTurn);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AEcoscapePlayerController::OnJumpPressed);
	InputComponent->BindAction("Jump", IE_Released, this, &AEcoscapePlayerController::OnJumpReleased);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AEcoscapePlayerController::OnCrouchPressed);
	InputComponent->BindAction("Crouch", IE_Released, this, &AEcoscapePlayerController::OnCrouchReleased);
}

void AEcoscapePlayerController::OnMoveForward(float Value)
{
	EcoscapeCharacter->AddMovementInput(EcoscapeCharacter->GetActorForwardVector(), Value);
}

void AEcoscapePlayerController::OnMoveRight(float Value)
{
	EcoscapeCharacter->AddMovementInput(EcoscapeCharacter->GetActorRightVector(), Value);
}

void AEcoscapePlayerController::OnLookUp(float Value)
{
	EcoscapeCharacter->AddControllerPitchInput(Value);
}

void AEcoscapePlayerController::OnTurn(float Value)
{
	EcoscapeCharacter->AddControllerYawInput(Value);
}

void AEcoscapePlayerController::OnJumpPressed()
{
	EcoscapeCharacter->Jump();
}

void AEcoscapePlayerController::OnJumpReleased()
{
	EcoscapeCharacter->StopJumping();
}

void AEcoscapePlayerController::OnCrouchPressed()
{
	EcoscapeCharacter->Crouch();
}

void AEcoscapePlayerController::OnCrouchReleased()
{
	EcoscapeCharacter->UnCrouch();
}

void AEcoscapePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
