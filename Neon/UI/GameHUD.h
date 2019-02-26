// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "Blueprint/UserWidget.h"
#include "GameHUD.generated.h"

/**
 * 
 */
UCLASS()
class NEON_API UGameHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	///////////////////////// Main API /////////////////////////
	void SetProgress(int32 PlayerIdx, float ProgressPercent);

	///////////////////////// Implemented by UMG_* classes /////////////////////////
	UFUNCTION(BlueprintImplementableEvent, Category = Ship)
	void SetRedProgress(float ProgressPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = Ship)
	void SetGreenProgress(float ProgressPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = Ship)
	void SetOrangeProgress(float ProgressPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = Ship)
	void SetBlueProgress(float ProgressPercent);	
};
