// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SeedFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSeed, Log, All);

UCLASS()
class NEON_API USeedFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = Seed, meta = (WorldContext = "WorldContextObject"))
	static int32 GetSeed(UObject *WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = Seed, meta = (WorldContext = "WorldContextObject"))
	static float GetSeedDifficulty(UObject *WorldContextObject);

	///////////////////////// Setters /////////////////////////
	UFUNCTION(BlueprintCallable, Category = Seed, meta = (WorldContext = "WorldContextObject"))
	static void RandomiseSeed(UObject *WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = Seed, meta = (WorldContext = "WorldContextObject"))
	static void SetSeedFromString(UObject *WorldContextObject, FString const &Input);

	UFUNCTION(BlueprintCallable, Category = Seed, meta = (WorldContext = "WorldContextObject"))
	static void SetSeedFromHexInputs(UObject *WorldContextObject, int32 Hex1, int32 Hex2, int32 Hex3, int32 Hex4, int32 Hex5, int32 Hex6, int32 Hex7, int32 Hex8);

	UFUNCTION(BlueprintCallable, Category = Seed, meta = (WorldContext = "WorldContextObject"))
	static void GetSplitIntsFromSeed(UObject *WorldContextObject, int32 &Hex1, int32 &Hex2, int32 &Hex3, int32 &Hex4, int32 &Hex5, int32 &Hex6, int32 &Hex7, int32 &Hex8);
};
