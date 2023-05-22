// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/AnimalUI.h"
#include "UI/Codex.h"
#include "EcoscapeGameModeBase.generated.h"

class UAnimalData;
class ABaseAnimal;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnimalHappinessUpdated, ABaseAnimal*, Animal, float, NewHappiness);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnyAnimalDies, ABaseAnimal*, Animal);

UCLASS(Blueprintable)
class UNotificationPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void AddNotification(const FText& Title, const FText& Content, float Time = 5, UTexture2D* Icon = nullptr);
};

USTRUCT()
struct FEcoscapeProgressionItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UAnimalData* Animal;
	UPROPERTY(EditAnywhere)
	FString Habitat;
	UPROPERTY(EditAnywhere)
	float MinimumHappiness = 0.7f;
};

UCLASS(Blueprintable)
class UEcoscapeProgression : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FEcoscapeProgressionItem> Stages;
};

/**
 * Base game mode for Ecoscape
 */
UCLASS(Blueprintable)
class ECOSCAPE_API AEcoscapeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEcoscapeGameModeBase();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContext"))
	static FORCEINLINE AEcoscapeGameModeBase* GetEcoscapeBaseGameMode(UObject* WorldContext)
	{
		return Cast<AEcoscapeGameModeBase>(UGameplayStatics::GetGameMode(WorldContext));
	}
	
	UFUNCTION(BlueprintCallable, Exec)
	void SpawnAnimalAtCursor(FString Animal);

	UPROPERTY(BlueprintAssignable)
	FOnAnimalHappinessUpdated AnimalHappinessUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnAnyAnimalDies AnimalDies;

	UPROPERTY(EditAnywhere)
	UEcoscapeProgression* Progression;

	UPROPERTY(BlueprintReadWrite)
	int CurrentProgressionStage = 0;

	UPROPERTY(BlueprintReadWrite)
	TMap<int, ABaseAnimal*> CurrentAnimals;

	UPROPERTY(BlueprintReadWrite)
	TArray<int> SickAnimals;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAnimalUnlockedWidget> AnimalUnlockedWidgetClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCodexUI> CodexWidgetClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCodexFeedUI> CodexFeedWidgetClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UNotificationPanel> NotificationsWidgetClass;
	
	UFUNCTION()
	void OnAnimalHappinessUpdated(ABaseAnimal* Animal, float NewHappiness);

	UFUNCTION()
	void OnAnimalDies(ABaseAnimal* Animal);

	UFUNCTION(BlueprintCallable, Exec)
	void UnlockNextAnimal();

	UFUNCTION(BlueprintCallable, Exec)
	void UnlockStage(int Stage);

	UFUNCTION(BlueprintCallable)
	void GiveCodexEntry(UCodexEntry* CodexEntry);

	UFUNCTION(BlueprintCallable)
	bool CanAnimalGetSick(ABaseAnimal* Animal);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetAnimalSicknessModifier() { return FMath::Clamp(1 - (SickAnimals.Num() / 5), 0, 1); }

	UPROPERTY(BlueprintReadWrite)
	TArray<UCodexEntry*> UnlockedCodexEntries;
	
	UPROPERTY(BlueprintReadOnly)
	UNotificationPanel* NotificationPanel;

	UFUNCTION(BlueprintCallable, Exec)
	void StartNewSave();

	UFUNCTION(BlueprintCallable)
	void AddWidgetToQueue(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsWidgetQueueEmpty() { return WidgetQueue.IsEmpty(); }

	UPROPERTY(BlueprintReadOnly)
	UCodexFeedUI* CodexFeed;

	UPROPERTY(BlueprintReadOnly)
	UCodexUI* Codex;
	
protected:
	UPROPERTY()
	TArray<UUserWidget*> WidgetQueue;

	UPROPERTY()
	UUserWidget* CurrentWidget = nullptr;
};
