// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "NeonGameMode_Arcade.h"

// Engine
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

// Game
#include "Pawns/NeonPawn.h"
#include "NeonGameInstance.h"
#include "UI/PlayerHUD.h"
#include "UI/GameHUD.h"
#include "UI/NewHighscore.h"
#include "Blueprint/HighscoreFunctionLibrary.h"

ANeonGameMode_Arcade::ANeonGameMode_Arcade()
{
	DefaultPawnClass = ANeonPawn::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
}

void ANeonGameMode_Arcade::BeginPlay() 
{	
	Super::BeginPlay();

	UE_LOG(LogNeon, Display, TEXT("Arcade mode began at %s"), *FDateTime::Now().ToString());

	// Set timer visible
	UPlayerHUD *PlayerHUD = GetPlayerHUD(0);
	GOOSE_BAIL(PlayerHUD);

	PlayerHUD->SetTimerVisible(true);

	// Show the highscore if available
	UNeonGameInstance *Instance = UNeonGameInstance::Get(this);
	GOOSE_BAIL(Instance);

	FHighscoreBoard &HighscoreBoard = Instance->SaveGame->GetHighscoreBoardForSeed(Instance->RandomSeed);

	if (HighscoreBoard.Num() > 0)	
	{	
		PlayerHUD->SetHighscoreVisible(true);
		PlayerHUD->SetHighscoreText(HighscoreBoard[0].Name, HighscoreBoard[0].Time);
	}
	else
	{
		PlayerHUD->SetHighscoreVisible(false);
	}
}

void ANeonGameMode_Arcade::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bPlayTimerIsRunning)
	{
		PlayTimer += DeltaSeconds;

		// Update UI
		UPlayerHUD *PlayerHUD = GetPlayerHUD(0);
		GOOSE_BAIL(PlayerHUD);

		PlayerHUD->SetTimer(UHighscoreFunctionLibrary::FormatTime(PlayTimer));
	}
}

void ANeonGameMode_Arcade::SetPixellationForPlayer(int32 PlayerIdx, float ResolutionPerc)
{
	GOOSE_BAIL(PostFX_Pixellate);

	float TargetParamValue = 500.f * ResolutionPerc * ResolutionPerc;

    static FName ParamNames[] =
    {
        FName(TEXT("ResolutionTL")),
        FName(TEXT("ResolutionTR")),
        FName(TEXT("ResolutionBL")),
        FName(TEXT("ResolutionBR"))
    };
    
	PostFX_Pixellate->SetScalarParameterValue(ParamNames[0], TargetParamValue);
    PostFX_Pixellate->SetScalarParameterValue(ParamNames[1], TargetParamValue);
    PostFX_Pixellate->SetScalarParameterValue(ParamNames[2], TargetParamValue);
    PostFX_Pixellate->SetScalarParameterValue(ParamNames[3], TargetParamValue);
}

bool ANeonGameMode_Arcade::CanPlayerRespawn(int32 PlayerIdx, class APawn *Pawn) const
{
	return false;
}

void ANeonGameMode_Arcade::PlayerDied_Implementation(int32 PlayerIdx) 
{
	UPlayerHUD *PlayerHUD = GetPlayerHUD(PlayerIdx);
	GOOSE_BAIL(PlayerHUD);

	PlayerHUD->SetTimerVisible(false);
}

class UUserWidget *ANeonGameMode_Arcade::CreateLevelCompleteUI(int32 PlayerIdx)
{
	bPlayTimerIsRunning = false;

	// Tell the player's HUD to flash the timer
	UPlayerHUD *PlayerHUD = GetPlayerHUD(PlayerIdx);
	GOOSE_BAIL_RETURN(PlayerHUD, nullptr);

	PlayerHUD->StopAndFlashTimer();
	
	// Get the current highscore, if any
	UNeonGameInstance *Instance = UNeonGameInstance::Get(this);
	GOOSE_BAIL_RETURN(Instance, nullptr);

	FHighscoreBoard &HighscoreBoard = Instance->SaveGame->GetHighscoreBoardForSeed(Instance->RandomSeed);
	int32 ThisRank = HighscoreBoard.GetPlace(PlayTimer) + 1;

	bool bAchievedHighscoreThisTime = ThisRank > 0; // 0 because GetPlace returns -1 for no good, and we +1
	
	if (bAchievedHighscoreThisTime)
	{
		UClass *UMG_NewHighscore = UGooseUtil::GetClass(TEXT("UMG_NewHighscore"), TEXT("UI/GameOver"));
		GOOSE_BAIL_RETURN(UMG_NewHighscore, nullptr);

		UNewHighscore *HighscoreUI = CreateWidget<UNewHighscore>(GetWorld(), UMG_NewHighscore);
		GOOSE_BAIL_RETURN(UMG_NewHighscore, nullptr);

		HighscoreUI->SetTime(PlayTimer);
		HighscoreUI->SetRank(ThisRank);

		return HighscoreUI;
	}
	else
	{
		// Create standard UI instead!
		return Super::CreateLevelCompleteUI(PlayerIdx);
	}
	
	// Create the UI and return
	return nullptr;	
}

UUserWidget *ANeonGameMode_Arcade::CreateGameHUD()
{
	UClass *UMG_GameHUD = UGooseUtil::GetClass(TEXT("UMG_GameHUD"), TEXT("UI/InGame"));
	GOOSE_BAIL_RETURN(UMG_GameHUD, nullptr);

	GameHUD = CreateWidget<UGameHUD>(GetWorld(), UMG_GameHUD);
	GOOSE_BAIL_RETURN(GameHUD, nullptr);

	GameHUD->AddToViewport();

	return GameHUD;
}
