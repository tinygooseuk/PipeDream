// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "SaveGame/NeonSaveGame.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HighscoreFunctionLibrary.generated.h"

UCLASS()
class NEON_API UHighscoreFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	///////////////////////// Highscore Utils /////////////////////////
	UFUNCTION(BlueprintCallable, meta=(WorldContext=WorldContextObject), Category = Highscores)
	static bool HasHighscores(UObject *WorldContextObject, int32 Seed);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = WorldContextObject), Category = Highscores)
	static int32 GetNumHighscores(UObject *WorldContextObject, int32 Seed);
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = WorldContextObject), Category = Highscores)
	static FHighscoreBoard GetHighscoreBoardCopy(UObject *WorldContextObject, int32 Seed);
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = WorldContextObject), Category = Highscores)
	static void SetHighscore(UObject *WorldContextObject, int32 Seed, FString Name, float Time);

	///////////////////////// Highscore Board Utils /////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Highscore Board")
	static int32 GetNumberOfEntries(FHighscoreBoard const &Board);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Highscore Board")
	static FHighscoreEntry GetEntry(FHighscoreBoard const &Board, int32 Index);

	///////////////////////// Formatting/breaking /////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = HighScores)
	static void BreakHighscore(FHighscoreEntry Entry, FString &Name, float &Time);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Highscores)
	static FText FormatTime(float TimeSeconds);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Highscores)
	static FString GetNameFromDigits(int32 Letter1, int32 Letter2, int32 Letter3);
};
