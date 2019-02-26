// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "Blueprint/UserWidget.h"
#include "LevelComplete.generated.h"

UCLASS()
class NEON_API ULevelComplete : public UUserWidget
{
	GENERATED_BODY()

public:
	///////////////////////// Implemented by UMG_* classes /////////////////////////
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Highscore)
	void SetHighlightedRank(int32 Rank);
};
