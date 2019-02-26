// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "NeonGameModeBase.h"

// Engine
#include "Components/AudioComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/AmbientSound.h"
#include "Sound/SoundBase.h"

// Game
#include "Pawns/NeonPawn.h"
#include "NeonGameInstance.h"
#include "UI/GameHUD.h"
#include "UI/PlayerHUD.h"

ANeonGameModeBase::ANeonGameModeBase()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = ANeonPawn::StaticClass();
}

void ANeonGameModeBase::BeginPlay()
{	
	CreatePostFXPixellate();
	CreateGameHUD();
	SetUpBGM();

	// Get the player controller iterator, and skip over the first player
	FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
	++It;

	// Remove each player!
	for (; It; ++It)
	{
		APlayerController *PC = It->Get();
		UGameplayStatics::RemovePlayer(PC, true);
	}

	UNeonGameInstance *Instance = UNeonGameInstance::Get(this);	GOOSE_BAIL(Instance);
	UE_LOG(LogNeon, Warning, TEXT("Bringing up session with seed: %08X"), Instance->RandomSeed);
}

void ANeonGameModeBase::SetPixellationForPlayer(int32 PlayerIdx, float ResolutionPerc)
{	
	GOOSE_BAIL(PostFX_Pixellate);	

	static FName ParamNames[] = 
	{
		FName(TEXT("ResolutionTL")),
		FName(TEXT("ResolutionTR")),
		FName(TEXT("ResolutionBL")),
		FName(TEXT("ResolutionBR"))
	};
	
	/*
		Slightly awful. The PostProcess shader only knows about the 4 quadrants (Resolution0..3).
		We must here change 1 or more screen quadrants, based on which player needs updating.
		A mapping is shown below, where the number is the player index and the position indicates
		the quadrant. I.e., in single player, P0 occupies all quadrants, but in 2-player P0
		occupies only the left 2

		1P: 00	2P: 01	3P: 00	4P: 01 
			00		01		12		23
	*/
	int32 NumPlayers = UNeonGameInstance::Get(this)->NumPlayers;
	float TargetParamValue = FMath::Max(512.f * ResolutionPerc * ResolutionPerc, 192.f);

	switch (NumPlayers)
	{
		case 1:
			PostFX_Pixellate->SetScalarParameterValue(ParamNames[0], TargetParamValue);
			PostFX_Pixellate->SetScalarParameterValue(ParamNames[1], TargetParamValue);
			PostFX_Pixellate->SetScalarParameterValue(ParamNames[2], TargetParamValue);
			PostFX_Pixellate->SetScalarParameterValue(ParamNames[3], TargetParamValue);
			break;

		case 2:
			switch (PlayerIdx)
			{
				case 0:
					PostFX_Pixellate->SetScalarParameterValue(ParamNames[0], TargetParamValue);
					PostFX_Pixellate->SetScalarParameterValue(ParamNames[2], TargetParamValue);
					break;
			
				case 1:
					PostFX_Pixellate->SetScalarParameterValue(ParamNames[1], TargetParamValue);
					PostFX_Pixellate->SetScalarParameterValue(ParamNames[3], TargetParamValue);
					break;

				default:
					UE_LOG(LogNeon, Error, TEXT("Unexpected player index - %i"), NumPlayers)
					break;
			}
			break;

		case 3:
			switch (PlayerIdx)
			{
				case 0:
					PostFX_Pixellate->SetScalarParameterValue(ParamNames[0], TargetParamValue);
					PostFX_Pixellate->SetScalarParameterValue(ParamNames[1], TargetParamValue);
					break;			

				case 1:
					PostFX_Pixellate->SetScalarParameterValue(ParamNames[2], TargetParamValue);
					break;

				case 2:
					PostFX_Pixellate->SetScalarParameterValue(ParamNames[3], TargetParamValue);
					break;

				default:
					UE_LOG(LogNeon, Error, TEXT("Unexpected player index - %i"), NumPlayers)
					break;
			}
			break;

		case 4:
			switch (PlayerIdx)
			{
				case 0:	PostFX_Pixellate->SetScalarParameterValue(ParamNames[0], TargetParamValue); break;
				case 1: PostFX_Pixellate->SetScalarParameterValue(ParamNames[1], TargetParamValue); break;	
				case 2: PostFX_Pixellate->SetScalarParameterValue(ParamNames[2], TargetParamValue); break;
				case 3: PostFX_Pixellate->SetScalarParameterValue(ParamNames[3], TargetParamValue); break;

				default:
					UE_LOG(LogNeon, Error, TEXT("Unexpected player index - %i"), NumPlayers)
					break;
			}
			break;

		default:
			UE_LOG(LogNeon, Error, TEXT("Unexpected number of players - %i"), NumPlayers)
			break;
	}
}

void ANeonGameModeBase::SetProgressForPlayer(int32 PlayerIdx, float ProgressPerc)
{
	if (GameHUD)
	{
		GameHUD->SetProgress(PlayerIdx, ProgressPerc);
	}
}

void ANeonGameModeBase::SetUpBGM()
{
#if WITH_EDITOR
	static bool const bDisableBGMInEditor = true;

	if (bDisableBGMInEditor)
	{
		return;
	}
#endif

	UWorld *World = GetWorld(); 
	GOOSE_BAIL(World);

	// Create BGM player!
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAmbientSound *BGMPlayer = World->SpawnActor<AAmbientSound>(); 
	GOOSE_BAIL(BGMPlayer);
	UAudioComponent *AudioComp = BGMPlayer->GetAudioComponent();
	GOOSE_BAIL(AudioComp);
	USoundBase *MusicFile = UGooseUtil::GetObject<USoundBase>(TEXT("Cephalopod"), TEXT("Music"));
	GOOSE_BAIL(MusicFile);

	AudioComp->SetSound(MusicFile);
	AudioComp->AttenuationOverrides.bSpatialize = false;
	AudioComp->Play();
}

UUserWidget *ANeonGameModeBase::CreateGameOverUI(int32 PlayerIdx)
{
	APlayerController *PC = UGameplayStatics::GetPlayerController(this, PlayerIdx);
	GOOSE_BAIL_RETURN(PC, nullptr);

	UClass *UMG_GameOver = UGooseUtil::GetClass(TEXT("UMG_GameOver"), TEXT("UI/GameOver"));
	GOOSE_BAIL_RETURN(UMG_GameOver, nullptr);

	UUserWidget *GameOverUI = CreateWidget<UUserWidget>(PC, UMG_GameOver);
	GOOSE_BAIL_RETURN(GameOverUI, nullptr);

	return GameOverUI;
}

UUserWidget *ANeonGameModeBase::CreateLevelCompleteUI(int32 PlayerIdx)
{
	APlayerController *PC = UGameplayStatics::GetPlayerController(this, PlayerIdx);
	GOOSE_BAIL_RETURN(PC, nullptr);

	UClass *UMG_LevelComplete = UGooseUtil::GetClass(TEXT("UMG_LevelComplete"), TEXT("UI/GameOver"));
	GOOSE_BAIL_RETURN(UMG_LevelComplete, nullptr);

	UUserWidget *LevelCompleteUI = CreateWidget<UUserWidget>(PC, UMG_LevelComplete);
	GOOSE_BAIL_RETURN(LevelCompleteUI, nullptr);

	return LevelCompleteUI;
}

void ANeonGameModeBase::SetHUDVisible(bool bNewVisible)
{
	GOOSE_BAIL(GameHUD);
	GameHUD->SetVisibility(bNewVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

UPlayerHUD *ANeonGameModeBase::GetPlayerHUD(int32 PlayerIdx) const
{
	APlayerController *PC = UGameplayStatics::GetPlayerController(this, PlayerIdx);
	GOOSE_BAIL_RETURN(PC, nullptr);

	ANeonPawn *Pawn = Cast<ANeonPawn>(PC->GetPawn());
	GOOSE_BAIL_RETURN(Pawn, nullptr);

	return Pawn->GetPlayerHUD();
}


void ANeonGameModeBase::SetPlayerHUDsVisible(bool bNewVisible)
{
	for (int32 Idx = 0; Idx < 4; Idx++)
	{
		UPlayerHUD *PlayerHUD = GetPlayerHUD(Idx);
		if (!PlayerHUD)
		{
			continue;
		}	

		PlayerHUD->SetVisibility(bNewVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ANeonGameModeBase::RequestPause(int32 PlayerIdx)
{	
	// Don't do anything for a plain NeonGameModeBase!!
	if (GetClass() == ANeonGameModeBase::StaticClass())
	{
		return;
	}

	SetPaused(true);
}

void ANeonGameModeBase::SetPaused(bool bNewPaused)
{
	if (bNewPaused == bIsGamePaused)
	{
		return;
	}

	if (bNewPaused)
	{
		UGameplayStatics::SetGamePaused(this, true);

		UClass *UMG_Pause = UGooseUtil::GetClass(TEXT("UMG_Pause"), TEXT("UI/InGame"));
		GOOSE_BAIL(UMG_Pause);

		PauseHUD = CreateWidget<UUserWidget>(GetWorld(), UMG_Pause);
		GOOSE_BAIL(PauseHUD);

		PauseHUD->AddToViewport();
	}
	else
	{
		UGameplayStatics::SetGamePaused(this, false);

		GOOSE_BAIL(PauseHUD);
		PauseHUD->RemoveFromParent();
	}

	bIsGamePaused = bNewPaused;
}

AActor *ANeonGameModeBase::ChoosePlayerStart_Implementation(AController *Player)
{
	APlayerController *PC = Cast<APlayerController>(Player);
	GOOSE_BAIL_RETURN(PC, nullptr);

	UWorld *World = GetWorld();
	GOOSE_BAIL_RETURN(World, nullptr);

	// Find player starts and order them if we haven't already (lazy caching)
	if (OrderedPlayerStarts.Num() == 0)
	{
		// Find all player starts and sort along the X axis
		OrderedPlayerStarts.Reserve(8);

		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			OrderedPlayerStarts.Add(*It);
		}

		OrderedPlayerStarts.Sort([](AActor const &PS1, AActor const &PS2)
		{
			return PS1.GetActorLocation().X > PS2.GetActorLocation().X;
		});
	}

	// Find out this player number
	uint32 PlayerIdx = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It, ++PlayerIdx)
	{
		if (Player == It->Get())
		{
			break;
		}
	}
	
	if (PlayerIdx >= (uint32)OrderedPlayerStarts.Num())
	{
		UE_LOG(LogNeon, Error, TEXT("Too many players/not enough player starts! (can't place player %u with %i player starts!)"), PlayerIdx, OrderedPlayerStarts.Num());
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	return OrderedPlayerStarts[PlayerIdx];
}

void ANeonGameModeBase::CreatePostFXPixellate()
{
	// Find the PostFX volume
	TActorIterator<APostProcessVolume> It(GetWorld());

	APostProcessVolume *PostFXVolume = *It;
	GOOSE_BAIL(PostFXVolume);

	// Find the base material
	UMaterial *BasePostMaterial = UGooseUtil::GetObject<UMaterial>(TEXT("PP_Pixellate"), TEXT("PostFX"));
	GOOSE_BAIL(BasePostMaterial);
	
	// Create the new material, set it up, and use it
	PostFX_Pixellate = UMaterialInstanceDynamic::Create(BasePostMaterial, PostFXVolume);
	GOOSE_BAIL(PostFX_Pixellate);

	PostFXVolume->AddOrUpdateBlendable(PostFX_Pixellate, 1.f);
}
