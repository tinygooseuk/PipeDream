// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "Blueprint/UserWidget.h"
#include "NewHighscore.generated.h"

UCLASS()
class NEON_API UNewHighscore : public UUserWidget
{
	GENERATED_BODY()
	
public:
	///////////////////////// Implemented by UMG_* classes /////////////////////////
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Highscore)
	void SetTime(float TimeSeconds);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Highscore)
	void SetRank(int32 Rank);
};
