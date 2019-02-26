// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "Blueprint/UserWidget.h"

// Engine
#include "Internationalization/Text.h"

#include "PlayerHUD.generated.h"

UCLASS()
class NEON_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// Vitals
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
	void SetHealth(float NewHealth);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
	void SetSpeed(float NewSpeed);

	// Position
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
	void SetPositionVisible(bool bNewVisible);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
	void SetPosition(int32 Index);

	// Timer
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
	void SetTimerVisible(bool bNewVisible);
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
	void StopAndFlashTimer();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
	void SetTimer(FText const &FormattedTime);

	// Highscore
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
	void SetHighscoreVisible(bool bNewVisible);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
	void SetHighscoreText(FString const &Name, float Highscore);
};
