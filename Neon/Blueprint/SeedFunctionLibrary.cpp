// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "SeedFunctionLibrary.h"

// Engine 
#include "Math/RandomStream.h"

// Game
#include "NeonGameInstance.h"
#include "Furniture/RandomPipeParameters.h"

DEFINE_LOG_CATEGORY(LogSeed)


int32 USeedFunctionLibrary::GetSeed(UObject *WorldContextObject)
{
	UNeonGameInstance *Instance = UNeonGameInstance::Get(WorldContextObject);
	GOOSE_BAIL_RETURN(Instance, 0x1BAD5EED);

	return Instance->RandomSeed;
}

float USeedFunctionLibrary::GetSeedDifficulty(UObject *WorldContextObject)
{
	int32 Seed = GetSeed(WorldContextObject);

	FRandomStream Random;
	Random.Initialize(Seed);

	FRandomPipeParameters PipeParams(Random);
	return PipeParams.EstimateDifficulty();
}

void USeedFunctionLibrary::RandomiseSeed(UObject *WorldContextObject)
{
	UNeonGameInstance *Instance = UNeonGameInstance::Get(WorldContextObject);
	GOOSE_BAIL(Instance);

	int32 RandomSeed = FMath::RandRange(0, TNumericLimits<int32>::Max()-1);

	//BUGFIX:	The FRand() implementation seems to favour always having 0
	//			on the end of the seed. So here, we bump it by 0..15 to compensate.
	RandomSeed += FMath::RandRange(0x0, 0xF);
	
	Instance->RandomSeed = RandomSeed;
}

void USeedFunctionLibrary::SetSeedFromString(UObject *WorldContextObject, FString const &Input)
{
	int32 Hash = (int32)FCrc::StrCrc32(*Input);

	UNeonGameInstance *Instance = UNeonGameInstance::Get(WorldContextObject);
	GOOSE_BAIL(Instance);

	Instance->RandomSeed = Hash;
}

void USeedFunctionLibrary::SetSeedFromHexInputs(UObject *WorldContextObject, int32 Hex1, int32 Hex2, int32 Hex3, int32 Hex4, int32 Hex5, int32 Hex6, int32 Hex7, int32 Hex8)
{
	UNeonGameInstance *Instance = UNeonGameInstance::Get(WorldContextObject);
	GOOSE_BAIL(Instance);

	Instance->RandomSeed =	(((int8)Hex1) << 28) +
							(((int8)Hex2) << 24) +
							(((int8)Hex3) << 20) +
							(((int8)Hex4) << 16) +
							(((int8)Hex5) << 12) +
							(((int8)Hex6) << 8) +
							(((int8)Hex7) << 4) +
							(((int8)Hex8) << 0);
}
void USeedFunctionLibrary::GetSplitIntsFromSeed(UObject *WorldContextObject, int32 &Hex1, int32 &Hex2, int32 &Hex3, int32 &Hex4, int32 &Hex5, int32 &Hex6, int32 &Hex7, int32 &Hex8)
{
	UNeonGameInstance *Instance = UNeonGameInstance::Get(WorldContextObject);
	GOOSE_BAIL(Instance);

	Hex1 = (Instance->RandomSeed >> 28) & 0xF;
	Hex2 = (Instance->RandomSeed >> 24) & 0xF;
	Hex3 = (Instance->RandomSeed >> 20) & 0xF;
	Hex4 = (Instance->RandomSeed >> 16) & 0xF;
	Hex5 = (Instance->RandomSeed >> 12) & 0xF;
	Hex6 = (Instance->RandomSeed >> 8) & 0xF;
	Hex7 = (Instance->RandomSeed >> 4) & 0xF;
	Hex8 = (Instance->RandomSeed >> 0) & 0xF;
}