// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "NeonGameModeBase.h"
#include "NeonGameMode_Race.generated.h"

UCLASS(MinimalAPI)
class ANeonGameMode_Race : public ANeonGameModeBase
{
	GENERATED_BODY()

public:
	ANeonGameMode_Race();

	void BeginPlay() override;
	void Tick(float DeltaSeconds);

	void UpdatePlayerPositions();
		
	///////////////////////// Superclass overrides //////////////////////////////
	bool CanPlayerRespawn(int32 PlayerIdx, class APawn *Pawn) const override;
	UUserWidget *CreateGameOverUI(int32 PlayerIdx) override;
	
	virtual bool IsRealGameMode() const { return true; }

protected:
	class UUserWidget *CreateGameHUD() override;

private:
	float PlayerPositionUpdateTimer = 0.f;
};



