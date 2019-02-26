// (c) 2017 TinyGoose Ltd., All Rights Reserved.
#pragma once

#include "Neon.h"
#include "Components/ActorComponent.h"
#include "PipeProgressComponent.generated.h"

UCLASS(Config=Game)
class NEON_API UPipeProgressComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPipeProgressComponent();
	
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    UFUNCTION() void UpdateProgress(int32 Progress, int32 Total, class APipeSystem *PipeSystem);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FPipeProgressUpdateDelegate, class APipeSystem *, PipeSystem);
    FPipeProgressUpdateDelegate OnPipeProgressUpdated;
	
	///////////////////////// Getters /////////////////////////
	FORCEINLINE int32 GetCurrentPipeIndex() const					{ return CurrentPipeIndex; }
	FORCEINLINE float GetCurrentPipeProgress() const				{ return CurrentPipeProgress; }
	FORCEINLINE class APipeSystem *GetLastSeenPipeSystem() const	{ return LastSeenPipeSystem; }

	UFUNCTION(BlueprintCallable, Category = "Pipe Progress")
	FORCEINLINE bool IsGoingTheWrongWay() const						{ return bWrongWay; }

protected:
	// Returns a number [0..1] where 0 = at CP1, and 0.99 = right next to CP2
	float GetFractionBetweenCheckpoints(int32 CP1, int32 CP2) const;

	// Returns a number [0..1] where 0 = at this CP, and 0.99 = right next to next CP
	float GetFractionToNextCheckpoint() const;
	
	// Updates the state of "wrong way" travel
	void UpdateWrongWayState();

	// Maintains track of the current pipe progress
	int32 CurrentPipeIndex = 0;
	float CurrentPipeProgress = 0.f;

	// Maintains track of the direction of movement
	bool bWrongWay = false;
	float MinPipeProgress = TNumericLimits<float>::Max();
	float MaxPipeProgress = 0.f;

	class APipeSystem *LastSeenPipeSystem = nullptr;
};
