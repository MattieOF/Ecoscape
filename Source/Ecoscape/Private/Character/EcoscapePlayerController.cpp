// copyright lololol

// Disable this warning as Rider thinks the input functions can be const,
// but the binding functions don't allow const functions
// ReSharper disable CppMemberFunctionMayBeConst

#include "Character/EcoscapePlayerController.h"

#include "EcoscapeLog.h"
#include "Kismet/GameplayStatics.h"

AEcoscapePlayerController::AEcoscapePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEcoscapePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Get character
	FPCharacter = Cast<AEcoscapeFPPlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	TDCharacter = GetWorld()->SpawnActor<AEcoscapeTDCharacter>(FVector(0, 0, 1000), FRotator::ZeroRotator);

	// Switch the view.
	SetView(EPSFirstPerson, true);

	// Setup input
	InputComponent->BindAxis("MoveForward", this, &AEcoscapePlayerController::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &AEcoscapePlayerController::OnMoveRight);
	InputComponent->BindAxis("LookUp", this, &AEcoscapePlayerController::OnLookUp);
	InputComponent->BindAxis("Turn", this, &AEcoscapePlayerController::OnTurn);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AEcoscapePlayerController::OnJumpPressed);
	InputComponent->BindAction("Jump", IE_Released, this, &AEcoscapePlayerController::OnJumpReleased);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AEcoscapePlayerController::OnCrouchPressed);
	InputComponent->BindAction("Crouch", IE_Released, this, &AEcoscapePlayerController::OnCrouchReleased);
	InputComponent->BindAction("SwitchView", IE_Pressed, this, &AEcoscapePlayerController::OnSwitchView);
}

void AEcoscapePlayerController::OnMoveForward(const float Value)
{
	CurrentPawn->AddMovementInput(CurrentView == EPSFirstPerson ? CurrentPawn->GetActorForwardVector() : FVector::ForwardVector, Value);
}

void AEcoscapePlayerController::OnMoveRight(const float Value)
{
	CurrentPawn->AddMovementInput(CurrentView == EPSFirstPerson ? CurrentPawn->GetActorRightVector() : FVector::RightVector, Value);
}	

void AEcoscapePlayerController::OnLookUp(const float Value)
{
	CurrentPawn->AddControllerPitchInput(Value);
}

void AEcoscapePlayerController::OnTurn(const float Value)
{
	CurrentPawn->AddControllerYawInput(Value);
}

void AEcoscapePlayerController::OnJumpPressed()
{
	if (CurrentView == EPSFirstPerson)
		FPCharacter->Jump();
}

void AEcoscapePlayerController::OnJumpReleased()
{
	if (CurrentView == EPSFirstPerson)
		FPCharacter->StopJumping();
}

void AEcoscapePlayerController::OnCrouchPressed()
{
	if (CurrentView == EPSFirstPerson)
		FPCharacter->Crouch();
}

void AEcoscapePlayerController::OnCrouchReleased()
{
	if (CurrentView == EPSFirstPerson)
		FPCharacter->UnCrouch();
}

void AEcoscapePlayerController::OnSwitchView()
{
	if (CurrentSwitchViewCooldown > 0)
		return;
	
	SetView(static_cast<EEcoscapePlayerView>(!CurrentView), false);
	
	CurrentSwitchViewCooldown = SwitchViewCooldown;
}

void AEcoscapePlayerController::SetView(const EEcoscapePlayerView NewView, const bool bInstant, const float BlendTime)
{
	// Set variables
	APawn* PreviousPawn = CurrentPawn;
	CurrentView = NewView;

	// Possess pawn
	if (CurrentView == EPSFirstPerson)
		Possess(FPCharacter);
	else if (CurrentView == EPSTopDown)
		Possess(TDCharacter);
	else
		UE_LOG(LogEcoscape, Error, TEXT("Invalid player view: %i!"), static_cast<int>(CurrentView));

	// Update view target
	CurrentPawn = GetPawn();
	SetViewTarget(PreviousPawn);
	SetViewTargetWithBlend(CurrentPawn, BlendTime, VTBlend_Cubic, 3, true);

	// Call blueprint events
	OnEcoscapePlayerViewChanged(CurrentView, CurrentPawn, bInstant ? 0 : BlendTime);
}

void AEcoscapePlayerController::SetMouseEnabled(const bool NewState)
{
	bShowMouseCursor = NewState;
	bEnableClickEvents = NewState;
	bEnableMouseOverEvents = NewState;
}

void AEcoscapePlayerController::CenterMouse()
{
	int ViewportWidth, ViewportHeight;
	GetViewportSize(ViewportWidth, ViewportHeight);
	SetMouseLocation(ViewportWidth / 2, ViewportHeight / 2);
}

void AEcoscapePlayerController::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Tick cooldowns
	if (CurrentSwitchViewCooldown > 0)
		CurrentSwitchViewCooldown = FMath::Max(CurrentSwitchViewCooldown - DeltaTime, 0);
}
