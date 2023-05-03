// copyright lololol

// Disable this warning as Rider thinks the input functions can be const,
// but the binding functions don't allow const functions
// ReSharper disable CppMemberFunctionMayBeConst

#include "Character/EcoscapePlayerController.h"

#include "EcoscapeGameInstance.h"
#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "Camera/CameraActor.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "World/EcoscapeTerrain.h"

AEcoscapePlayerController::AEcoscapePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

bool AEcoscapePlayerController::GoToTerrain(AEcoscapeTerrain* Terrain)
{
	if (!FPCharacter || !TDCharacter)
		return false;

	if (!Terrain)
	{
		UE_LOG(LogEcoscape, Error, TEXT("Null terrain provided to AEcoscapePlayerController::GoToTerrain"));
		return false;
	}
	
	CurrentTerrain = Terrain;
	FVector TerrainCenter = Terrain->GetCenterPosition();
	FPCharacter->SetActorLocation(TerrainCenter + FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(FPCharacter)));
	TerrainCenter.Z = TopDownSpawnHeight;
	TDCharacter->SetActorLocation(TerrainCenter);
	TDCharacter->GoToTerrain(Terrain);
	SetView(EPSFirstPerson, true, 0);

	return true;
}

void AEcoscapePlayerController::GoToCursor()
{
	if (CurrentView != EPSTopDown)
	{
		UE_LOG(LogEcoscape, Error, TEXT("You must be in top down mode to use GoToCursor!"));
		return;
	}
	
	FHitResult Hit;
	if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(this), ECC_WorldStatic, true, Hit, TArray<AActor*>()))
	{
		FPCharacter->SetActorLocation(Hit.ImpactPoint + FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(FPCharacter)));
		SetView(EPSFirstPerson, true);
	}
}

void AEcoscapePlayerController::GoToHabitatSelect()
{
	if (CurrentView == EPSMenu)
		return;
	
	SetView(EPSMenu, false);
	SetViewTarget(HabitatCam);
	CreateWidget(this, HabitatSelectUIClass)->AddToViewport();
}

void AEcoscapePlayerController::PlayMusic(USoundWave* NewMusic)
{
	if (CurrentMusic)
	{
		CurrentMusic->FadeOut(1.5f, 0);
		FadingMusic.Add({CurrentMusic, 1.5f});
	}

	CurrentMusic = Cast<UAudioComponent>(AddComponentByClass(UAudioComponent::StaticClass(), false, FTransform(), false));
	CurrentMusic->SetSound(NewMusic);
	CurrentMusic->bAllowSpatialization = false;
	CurrentMusic->Play();
	CurrentMusic->FadeIn(1.3f);
}

void AEcoscapePlayerController::BeginPlay()
{
	InteractionPrompt = CreateWidget<UInteractionPrompt>(this, InteractionPromptClass);
	InteractionPrompt->AddToViewport();
	
	// Get character
	FPCharacter = Cast<AEcoscapeFPPlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	TDCharacter = GetWorld()->SpawnActor<AEcoscapeTDCharacter>(TopDownCharacterClass, FVector(0, 0, TopDownSpawnHeight), FRotator::ZeroRotator);

	// Get habitat cam
	TArray<AActor*> HabitatCams;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), "HabitatCam", HabitatCams);
	HabitatCam = Cast<ACameraActor>(HabitatCams[0]);
	
	// Setup input
	InputComponent->BindAxis("MoveForward", this, &AEcoscapePlayerController::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &AEcoscapePlayerController::OnMoveRight);
	InputComponent->BindAxis("LookUp", this, &AEcoscapePlayerController::OnLookUp);
	InputComponent->BindAxis("Turn", this, &AEcoscapePlayerController::OnTurn);
	InputComponent->BindAxis("Scroll", this, &AEcoscapePlayerController::OnScroll);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AEcoscapePlayerController::OnJumpPressed);
	InputComponent->BindAction("Jump", IE_Released, this, &AEcoscapePlayerController::OnJumpReleased);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AEcoscapePlayerController::OnCrouchPressed);
	InputComponent->BindAction("Crouch", IE_Released, this, &AEcoscapePlayerController::OnCrouchReleased);
	InputComponent->BindAction("Modifier", IE_Pressed, this, &AEcoscapePlayerController::OnModifierPressed);
	InputComponent->BindAction("Modifier", IE_Released, this, &AEcoscapePlayerController::OnModifierReleased);
	InputComponent->BindAction("SwitchView", IE_Pressed, this, &AEcoscapePlayerController::OnSwitchView);
	InputComponent->BindAction("Interact", IE_Pressed, this, &AEcoscapePlayerController::OnInteract);
	InputComponent->BindAction("UseTool", IE_Pressed, this, &AEcoscapePlayerController::OnUseTool);
	InputComponent->BindAction("UseTool", IE_Released, this, &AEcoscapePlayerController::OnStopUseTool);
	InputComponent->BindAction("UseAltTool", IE_Pressed, this, &AEcoscapePlayerController::OnUseAltTool);
	InputComponent->BindAction("UseAltTool", IE_Released, this, &AEcoscapePlayerController::OnStopUseAltTool);
	InputComponent->BindAction("ResetTool", IE_Pressed, this, &AEcoscapePlayerController::OnResetTool);
	InputComponent->BindAction("HabitatSelect", IE_Pressed, this, &AEcoscapePlayerController::GoToHabitatSelect);
	
	Super::BeginPlay();
	
	// Go to habitat select for now
	GoToTerrain(UEcoscapeGameInstance::GetEcoscapeGameInstance(GetWorld())->GetTerrain("Forest"));
	SetupHUD(); // Order here is important. If HUD is setup after going to habitat select, HUD will stay on screen 
	GoToHabitatSelect();
}

void AEcoscapePlayerController::OnMoveForward(const float Value)
{
	if (CurrentView == EPSMenu)
		return;
	
	CurrentPawn->AddMovementInput(CurrentView == EPSFirstPerson ? CurrentPawn->GetActorForwardVector() : FVector::ForwardVector, bModifierPressed ? Value * SprintSpeedModifier : Value);
}

void AEcoscapePlayerController::OnMoveRight(const float Value)
{
	if (CurrentView == EPSMenu)
		return;
	
	CurrentPawn->AddMovementInput(CurrentView == EPSFirstPerson ? CurrentPawn->GetActorRightVector() : FVector::RightVector, bModifierPressed ? Value * SprintSpeedModifier : Value);
}	

void AEcoscapePlayerController::OnLookUp(const float Value)
{
	if (CurrentView == EPSMenu)
		return;
	
	CurrentPawn->AddControllerPitchInput(Value);
}

void AEcoscapePlayerController::OnTurn(const float Value)
{
	if (CurrentView == EPSMenu)
		return;
	
	CurrentPawn->AddControllerYawInput(Value);
}

void AEcoscapePlayerController::OnScroll(float Value)
{
	if (CurrentView == EPSTopDown)
		TDCharacter->AddScrollInput(Value);
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

void AEcoscapePlayerController::OnModifierPressed()
{
	bModifierPressed = true;	
}

void AEcoscapePlayerController::OnModifierReleased()
{
	bModifierPressed = false;
}

void AEcoscapePlayerController::OnSwitchView()
{
	if (CurrentView == EPSMenu) // Don't switch if in the menu, some button will do that for us
		return;
	
	if (CurrentSwitchViewCooldown > 0)
		return;
	
	SetView(static_cast<EEcoscapePlayerView>(!CurrentView), false);
	
	CurrentSwitchViewCooldown = SwitchViewCooldown;
}

void AEcoscapePlayerController::OnUseTool()
{
	bUseDown = true;
	
	if (CurrentView == EPSTopDown)
		TDCharacter->OnToolUsed();
}

void AEcoscapePlayerController::OnStopUseTool()
{
	bUseDown = false;
}

void AEcoscapePlayerController::OnUseAltTool()
{
	bAltUseDown = true;
	
	if (CurrentView == EPSTopDown)
		TDCharacter->OnToolAltUsed();
}

void AEcoscapePlayerController::OnStopUseAltTool()
{
	bAltUseDown = false;
}

void AEcoscapePlayerController::OnResetTool()
{
	if (CurrentView == EPSTopDown)
		TDCharacter->ResetTool();
}

void AEcoscapePlayerController::OnInteract()
{
	if (CurrentView == EPSFirstPerson)
		FPCharacter->Interact();
}

void AEcoscapePlayerController::SetView(const EEcoscapePlayerView NewView, const bool bInstant, const float BlendTime)
{
	// Set variables
	APawn* PreviousPawn = CurrentPawn;
	CurrentView = NewView;

	if (NewView != EPSMenu)
	{
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
		SetViewTargetWithBlend(CurrentPawn, BlendTime, VTBlend_Cubic, 2, true);
	}

	// Call blueprint events
	OnPlayerViewChanged.Broadcast(NewView);
	OnEcoscapePlayerViewChanged(CurrentView, CurrentPawn, bInstant ? 0 : BlendTime);
}

void AEcoscapePlayerController::SetMouseEnabled(const bool NewState)
{
	if (NewState)
	{
		auto InputMode = FInputModeGameAndUI();
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
	}
	else
		SetInputMode(FInputModeGameOnly());

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

	// Check for music
	for (int i = FadingMusic.Num(); i--;)
	{
		FadingMusic[i].RemainingTime -= DeltaTime;
		if (FadingMusic[i].RemainingTime <= 0)
		{
			FadingMusic[i].AudioComponent->DestroyComponent();
			FadingMusic.RemoveAt(i);
		}
	}
}
