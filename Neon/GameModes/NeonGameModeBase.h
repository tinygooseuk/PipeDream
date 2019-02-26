// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "GameFramework/GameModeBase.h"
#include "NeonGameModeBase.generated.h"

UCLASS(MinimalAPI)
class ANeonGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANeonGameModeBase();

	void BeginPlay() override;

	///////////////////////// Override points /////////////////////////
	// Pixellate a player's screen
	UFUNCTION()
	virtual void SetPixellationForPlayer(int32 PlayerIdx, float ResolutionPerc);

	// Set a given player's progress slider
	virtual void SetProgressForPlayer(int32 PlayerIdx, float ProgressPerc);

	///////////////////////// Music ////////////////////
	// Set up the BGM for the game
	virtual void SetUpBGM();

	///////////////////////// Widgets /////////////////////////
	// Create the widget for game over display, for the given player
	virtual class UUserWidget *CreateGameOverUI(int32 PlayerIdx);

	// Create the widget for level complete display, for the given player
	virtual class UUserWidget *CreateLevelCompleteUI(int32 PlayerIdx);

	// Hide or show the HUD
	UFUNCTION(BlueprintCallable, Category = HUD)
	void SetHUDVisible(bool bNewVisible);

	// Get a given player pawn's HUD
	UFUNCTION(BlueprintCallable, Category = HUD)
	class UPlayerHUD *GetPlayerHUD(int32 PlayerIdx) const;

	UFUNCTION(BlueprintCallable, Category = HUD)
	void SetPlayerHUDsVisible(bool bNewVisible);

	///////////////////////// "Events" /////////////////////////
	UFUNCTION(BlueprintNativeEvent, Category = "Player Events")
	void PlayerDied(int32 PlayerIdx);
	virtual void PlayerDied_Implementation(int32 PlayerIdx) { }
	
	// Pausing
	virtual void RequestPause(int32 PlayerIdx);

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void SetPaused(bool bNewPaused);

	///////////////////////// Subclassers must implement! /////////////////////////
	virtual bool CanPlayerRespawn(int32 PlayerIdx, class APawn *Pawn) const { return false; }
	virtual bool IsRealGameMode() const { return false; }

protected:
	// Select an appropriate playerstart for this player
	AActor *ChoosePlayerStart_Implementation(AController *Player) override;

	///////////////////////// Post Effects /////////////////////////
	void CreatePostFXPixellate();

	///////////////////////// HUD/UI /////////////////////////
	virtual class UUserWidget *CreateGameHUD() { return nullptr; }

	///////////////////////// State /////////////////////////
	UPROPERTY() class UMaterialInstanceDynamic *PostFX_Pixellate = nullptr;
	UPROPERTY() class UGameHUD *GameHUD = nullptr;
	UPROPERTY() class UUserWidget *PauseHUD = nullptr;

	TArray<AActor *> OrderedPlayerStarts;

	bool bIsGamePaused = false;
};



