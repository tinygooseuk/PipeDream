// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "NeonGameModeBase.h"
#include "NeonGameMode_Arcade.generated.h"

UCLASS(MinimalAPI)
class ANeonGameMode_Arcade : public ANeonGameModeBase
{
	GENERATED_BODY()

public:
	ANeonGameMode_Arcade();

	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;

	void SetPixellationForPlayer(int32 PlayerIdx, float ResolutionPerc) override;
	
	///////////////////////// Override Points /////////////////////////
	bool CanPlayerRespawn(int32 PlayerIdx, class APawn *Pawn) const override;
	void PlayerDied_Implementation(int32 PlayerIdx) override;

	class UUserWidget *CreateLevelCompleteUI(int32 PlayerIdx) override;

	virtual bool IsRealGameMode() const { return true; }

protected:
	class UUserWidget *CreateGameHUD() override;

private:
	bool bPlayTimerIsRunning = true;
	float PlayTimer = 0.f;
};



