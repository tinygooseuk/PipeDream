// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "NeonGameInstance.h"

// Engine
#include "Kismet/GameplayStatics.h"

// Game
#include "Blueprint/SeedFunctionLibrary.h"
#include "SaveGame/NeonSaveGame.h"

UNeonGameInstance::UNeonGameInstance()
	: NumPlayers(1)
	, RandomSeed(0x12345678)
{
}

void UNeonGameInstance::Init()
{
	USeedFunctionLibrary::RandomiseSeed(this);

	SaveGame = UNeonSaveGame::Load();
}

void UNeonGameInstance::Shutdown()
{
	GOOSE_BAIL(SaveGame);
	SaveGame->Save();
}

UNeonGameInstance *UNeonGameInstance::Get(UObject const *WorldContextObject)
{
	return Cast<UNeonGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
}
