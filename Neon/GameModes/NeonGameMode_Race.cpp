// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "NeonGameMode_Race.h"

// Engine
#include "Kismet/GameplayStatics.h"
#include "Engine.h"

// Game
#include "Components/PipeProgressComponent.h"
#include "NeonGameInstance.h"
#include "Pawns/NeonPawn.h"
#include "UI/PlayerHUD.h"
#include "UI/GameHUD.h"

namespace
{
	constexpr float POSITION_UPDATE_INTERVAL = 0.2f;
}

ANeonGameMode_Race::ANeonGameMode_Race()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = ANeonPawn::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
}

void ANeonGameMode_Race::BeginPlay()
{	
	Super::BeginPlay();

	UNeonGameInstance *Instance = UNeonGameInstance::Get(this);
	
	UE_LOG(LogNeon, Display, TEXT("Race mode began at %s"), *FDateTime::Now().ToString());
	UE_LOG(LogNeon, Display, TEXT("Race is between %i local players"), Instance->NumPlayers);	

	// Create additional players as needed
	for (int32 i = 0; i < Instance->NumPlayers - 1; i++)
	{
		UE_LOG(LogNeon, Warning, TEXT("Creating player %i"), i+1);
		UGameplayStatics::CreatePlayer(this, i + 1);
	}	

	// Enable race UI for each player
	for (int32 i = 0; i < Instance->NumPlayers; i++)
	{
		UPlayerHUD *PlayerHUD = GetPlayerHUD(i);
		GOOSE_BAIL_CONTINUE(PlayerHUD);

		PlayerHUD->SetPositionVisible(true);
	}
}

void ANeonGameMode_Race::Tick(float DeltaSeconds)
{
	PlayerPositionUpdateTimer += DeltaSeconds;
	if (PlayerPositionUpdateTimer > POSITION_UPDATE_INTERVAL)
	{
		PlayerPositionUpdateTimer = 0.f;
		UpdatePlayerPositions();
	}
}

void ANeonGameMode_Race::UpdatePlayerPositions()
{
	UNeonGameInstance *Instance = UNeonGameInstance::Get(this);

	using FPositionEntry = TPair<int32, float>;

	TArray<FPositionEntry> CheckpointIndexes;
	CheckpointIndexes.SetNumZeroed(Instance->NumPlayers);

	// Collect each player's index
	for (int32 i = 0; i < Instance->NumPlayers; i++)
	{
		APlayerController *PC = UGameplayStatics::GetPlayerController(this, i);
		GOOSE_BAIL_CONTINUE(PC);

		ANeonPawn *NeonPawn = Cast<ANeonPawn>(PC->GetPawn());
		GOOSE_BAIL_CONTINUE(NeonPawn);

		UPlayerHUD *PlayerHUD = NeonPawn->GetPlayerHUD();
		GOOSE_BAIL_CONTINUE(PlayerHUD);

		UPipeProgressComponent *ProgressComponent = NeonPawn->FindComponentByClass<UPipeProgressComponent>();
		GOOSE_BAIL_CONTINUE(ProgressComponent);

		CheckpointIndexes[i] = FPositionEntry(i, ProgressComponent->GetCurrentPipeProgress());
	}

	// Sort array of indices
	CheckpointIndexes.Sort([](FPositionEntry const &P1, FPositionEntry const &P2) {
		return P1.Value > P2.Value;
	});

	// Assign positions!
	int32 Position = 0;
	float LastSeenProgress = TNumericLimits<float>::Max();
	for (int32 i = 0; i < CheckpointIndexes.Num(); i++)
	{
		static const bool bDebugPositions = false;

		if (bDebugPositions)
		{
			FString EntryString = FString::Printf(TEXT("[%i] Player %i @ %.2f"), i + 1, CheckpointIndexes[i].Key + 1, CheckpointIndexes[i].Value);
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.2f, FColor::White, EntryString, false, FVector2D(5.f, 5.f));
		}

		// Increment position if further through than the last one
		if (CheckpointIndexes[i].Value < LastSeenProgress)
		{
			LastSeenProgress = CheckpointIndexes[i].Value;
			Position++;
		}

		UPlayerHUD *PlayerHUD = GetPlayerHUD(CheckpointIndexes[i].Key);
		GOOSE_BAIL_CONTINUE(PlayerHUD);

		PlayerHUD->SetPosition(Position);
	}
}

bool ANeonGameMode_Race::CanPlayerRespawn(int32 PlayerIdx, APawn *Pawn) const 
{
	return true;
}

UUserWidget *ANeonGameMode_Race::CreateGameOverUI(int32 PlayerIdx)
{
	APlayerController *PC = UGameplayStatics::GetPlayerController(this, PlayerIdx);
	GOOSE_BAIL_RETURN(PC, nullptr);

	UClass *UMG_GameOver = UGooseUtil::GetClass(TEXT("UMG_RaceDied"), TEXT("UI/GameOver"));
	GOOSE_BAIL_RETURN(UMG_GameOver, nullptr);

	UUserWidget *GameOverUI = CreateWidget<UUserWidget>(PC, UMG_GameOver);
	GOOSE_BAIL_RETURN(GameOverUI, nullptr);

	return GameOverUI;
}

UUserWidget *ANeonGameMode_Race::CreateGameHUD()
{
	UClass *UMG_GameHUD = UGooseUtil::GetClass(TEXT("UMG_GameHUD"), TEXT("UI/InGame"));
	GOOSE_BAIL_RETURN(UMG_GameHUD, nullptr);

	GameHUD = CreateWidget<UGameHUD>(GetWorld(), UMG_GameHUD);
	GOOSE_BAIL_RETURN(GameHUD, nullptr);

	GameHUD->AddToViewport();

	return GameHUD;
}