// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "NeonSaveGame.h"

// Engine
#include "Kismet/GameplayStatics.h"

///////////////////////// FHighscoreBoard /////////////////////////
int32 FHighscoreBoard::GetPlace(float Time) const
{
	// Copy main list
	TArray<FHighscoreEntry> TempEntries;
	TempEntries.Append(List);
	
	// Create & add new entry
	FHighscoreEntry TestEntry;
	TestEntry.Name = "_Placement Test_";
	TestEntry.Time = Time;
	TempEntries.Add(TestEntry);

	// Sort by time - lower is earlier of course
	TempEntries.Sort([](FHighscoreEntry const &A, FHighscoreEntry const &B) -> bool
	{
		return A.Time < B.Time;
	});

	// Remove any above 10
	while (TempEntries.Num() > MAX_HIGHSCORES)
	{
		TempEntries.Pop();
	}

	// Find our testing entry in the array
	for (int32 Idx = 0; Idx < TempEntries.Num(); Idx++)
	{
		if (TempEntries[Idx].Name == TestEntry.Name)
		{
			return Idx;
		}
	}

	return -1;
}

bool FHighscoreBoard::AddScore(FHighscoreEntry Entry)
{
	// Work out if we even placed
	int32 Idx = GetPlace(Entry.Time);
	if (Idx == -1)
	{
		return false; // no place
	}

	// Add new entry
	List.Add(Entry);

	// Sort all entries
	SortHighscores();

	return true;
}

void FHighscoreBoard::SortHighscores(bool bAlsoClip)
{
	// Sort by time - lower is earlier of course
	List.Sort([](FHighscoreEntry const &A, FHighscoreEntry const &B) -> bool
	{
		return A.Time < B.Time;
	});

	// If clipping, remove any items > 10th
	if (bAlsoClip)
	{
		while (List.Num() > MAX_HIGHSCORES)
		{
			List.Pop(true);
		}
	}
}

FHighscoreEntry &FHighscoreBoard::operator[](int32 Idx)
{
	return List[Idx];
}
FHighscoreEntry const &FHighscoreBoard::operator[](int32 Idx) const
{
	return List[Idx];
}
int32 FHighscoreBoard::Num() const
{
	return List.Num();
}

///////////////////////// UNeonSaveGame /////////////////////////
UNeonSaveGame::UNeonSaveGame()
{
	
}

UNeonSaveGame *UNeonSaveGame::Load()
{
	// First, try to load from slot
	UNeonSaveGame *Settings = Cast<UNeonSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("Highscores"), 0));

	// Then, try to create one & save it
	if (!Settings)
	{
		Settings = Cast<UNeonSaveGame>(UGameplayStatics::CreateSaveGameObject(UNeonSaveGame::StaticClass()));
	}

	GOOSE_CHECK(Settings);
	return Settings;
}

int32 UNeonSaveGame::GetNumHighscoresForSeed(int32 Seed) const
{
	FHighscoreBoard const *Entry = HighscoreBoards.Find(Seed);
	GOOSE_BAIL_RETURN(Entry, 0);

	return Entry->Num();
}

FHighscoreBoard &UNeonSaveGame::GetHighscoreBoardForSeed(int32 Seed) 
{
	FHighscoreBoard *Board = HighscoreBoards.Find(Seed);

	if (!Board)
	{
		return HighscoreBoards.Add(Seed);
	}

	return *Board;
}

void UNeonSaveGame::SetHighscoreForSeed(int32 Seed, FHighscoreEntry NewEntry)
{
	FHighscoreBoard *MapEntry = HighscoreBoards.Find(Seed);
	if (!MapEntry)
	{
		FHighscoreBoard NewBoard;
		NewBoard.AddScore(NewEntry);

		HighscoreBoards.Add(Seed, NewBoard);
	}
	else
	{		
		FHighscoreBoard &Board = *MapEntry;
		Board.AddScore(NewEntry);
	}
}


void UNeonSaveGame::Save()
{
	UGameplayStatics::SaveGameToSlot(this, TEXT("Highscores"), 0);
}