// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "GameFramework/SaveGame.h"
#include "NeonSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FHighscoreEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Highscore)
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = Highscore)
	float Time;
};

USTRUCT(BlueprintType)
struct FHighscoreBoard
{
	GENERATED_BODY()

	static constexpr int32 MAX_HIGHSCORES = 10;

	// Work out where "Time" would place, or -1 if not placed
	int32 GetPlace(float Time) const;

	// Add a new entry and sort the scores
	bool AddScore(FHighscoreEntry Entry);

	// Sort the board's scores - use this when adding a new one
	void SortHighscores(bool bAlsoClip = true);

	// Operators
	FHighscoreEntry &operator[](int32 Idx);
	FHighscoreEntry const &operator[](int32 Idx) const;

	// Getters
	int32 Num() const;

private:
	///////////////////////// Members /////////////////////////
	UPROPERTY()
	TArray<FHighscoreEntry> List;
};

UCLASS(MinimalAPI)
class UNeonSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UNeonSaveGame();

	///////////////////////// Static getter /////////////////////////
	static UNeonSaveGame *Load();
	
	///////////////////////// Functions /////////////////////////
	// True if a given seed has at least 1 highscore
	FORCEINLINE bool HasHighscoresForSeed(int32 Seed) const { return GetNumHighscoresForSeed(Seed) > 0; }

	// True if a given seed has a highscore
	int32 GetNumHighscoresForSeed(int32 Seed) const;

	// Get the highscore for a given seed
	FHighscoreBoard &GetHighscoreBoardForSeed(int32 Seed);

	// Set the highscore for a given seed
	void SetHighscoreForSeed(int32 Seed, FHighscoreEntry NewEntry);

	// Save the savegame to disk
	void Save();

private:
	UPROPERTY()
	TMap<int32, FHighscoreBoard> HighscoreBoards;
};



