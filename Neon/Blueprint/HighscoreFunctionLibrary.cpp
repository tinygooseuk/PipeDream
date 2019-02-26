// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "HighscoreFunctionLibrary.h"

// Engine 

// Game
#include "SaveGame/NeonSaveGame.h"
#include "NeonGameInstance.h"


bool UHighscoreFunctionLibrary::HasHighscores(UObject *WorldContextObject, int32 Seed)
{
	UNeonGameInstance *Instance = UNeonGameInstance::Get(WorldContextObject);
	GOOSE_BAIL_RETURN(Instance, false);
	
	return Instance->SaveGame->HasHighscoresForSeed(Seed);
}

int32 UHighscoreFunctionLibrary::GetNumHighscores(UObject *WorldContextObject, int32 Seed)
{
	UNeonGameInstance *Instance = UNeonGameInstance::Get(WorldContextObject);
	GOOSE_BAIL_RETURN(Instance, 0);

	return Instance->SaveGame->GetNumHighscoresForSeed(Seed);
}

FHighscoreBoard UHighscoreFunctionLibrary::GetHighscoreBoardCopy(UObject *WorldContextObject, int32 Seed)
{
	UNeonGameInstance *Instance = UNeonGameInstance::Get(WorldContextObject);
	GOOSE_BAIL_RETURN(Instance, FHighscoreBoard());

	return Instance->SaveGame->GetHighscoreBoardForSeed(Seed);
}

void UHighscoreFunctionLibrary::SetHighscore(UObject *WorldContextObject, int32 Seed, FString Name, float Time)
{
	UNeonGameInstance *Instance = UNeonGameInstance::Get(WorldContextObject);
	GOOSE_BAIL(Instance);

	FHighscoreEntry NewEntry;
	NewEntry.Name = Name;
	NewEntry.Time = Time;

	Instance->SaveGame->SetHighscoreForSeed(Seed, NewEntry);
	Instance->SaveGame->Save();
}


int32 UHighscoreFunctionLibrary::GetNumberOfEntries(FHighscoreBoard const &Board)
{
	return Board.Num();
}
FHighscoreEntry UHighscoreFunctionLibrary::GetEntry(FHighscoreBoard const &Board, int32 Index)
{
	return Board[Index];
}


void UHighscoreFunctionLibrary::BreakHighscore(FHighscoreEntry Entry, FString &Name, float &Time)
{
	Name = Entry.Name;
	Time = Entry.Time;
}

FString UHighscoreFunctionLibrary::GetNameFromDigits(int32 Letter1, int32 Letter2, int32 Letter3)
{
	char Char1 = (char)Letter1 + 'A';
	char Char2 = (char)Letter2 + 'A';
	char Char3 = (char)Letter3 + 'A';

	return FString::Printf(TEXT("%c%c%c"), Char1, Char2, Char3);
}

FText UHighscoreFunctionLibrary::FormatTime(float TimeSeconds)
{
	float Minutes = FMath::FloorToFloat(TimeSeconds / 60.f);
	float SecondsAndMillis = FMath::Fmod(TimeSeconds, 60.f);

	float TotalSeconds;
	float Nanos = FMath::Modf(TimeSeconds, &TotalSeconds) * 100.f;

	float Seconds = TotalSeconds - (Minutes * 60.f);

	FText FormattedTime = FText::FromString(FString::Printf(TEXT("%02.0f:%02.0f.%02.0f"), Minutes, Seconds, Nanos));
	return FormattedTime;
}