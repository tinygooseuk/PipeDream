// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "PipeProgressComponent.h"

// Engine
#include "Engine/Engine.h"

// Game
#include "Furniture/PipeSystem.h"


UPipeProgressComponent::UPipeProgressComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPipeProgressComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPipeProgressComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) 
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		
	// Store the current pipe fraction
	CurrentPipeProgress = float(CurrentPipeIndex) + GetFractionToNextCheckpoint();
	
	// Update whether or not we're going the right way!
	UpdateWrongWayState();
}

void UPipeProgressComponent::UpdateProgress(int32 NewProgress, int32 Total, class APipeSystem *PipeSystem)
{
	if (LastSeenPipeSystem != PipeSystem)
	{
		CurrentPipeIndex = NewProgress;
		MinPipeProgress = TNumericLimits<float>::Max();
		MaxPipeProgress = 0.f;

		LastSeenPipeSystem = PipeSystem;
	}
	else
	{
		CurrentPipeIndex = FMath::Max(NewProgress, CurrentPipeIndex);
	}

	OnPipeProgressUpdated.ExecuteIfBound(PipeSystem);
}

float UPipeProgressComponent::GetFractionBetweenCheckpoints(int32 CP1, int32 CP2) const
{
	// No pipe system? 0 then!
	if (!LastSeenPipeSystem)
	{
		return 0.f;
	}

	AActor *Owner = GetOwner();
	GOOSE_BAIL_RETURN(Owner, 0.f);

	// Get the current & next CP positions
	FVector CurrentCP = LastSeenPipeSystem->GetCheckpointLocation(CP1);
	FVector NextCP = LastSeenPipeSystem->GetCheckpointLocation(CP2);

	FVector Position = Owner->GetActorLocation();

	float MyDistanceToNext = FVector::Distance(Position, NextCP);
	float MaxDistanceToNext = FVector::Distance(CurrentCP, NextCP);

	return 1.f - MyDistanceToNext / MaxDistanceToNext;
}

float UPipeProgressComponent::GetFractionToNextCheckpoint() const
{
	return GetFractionBetweenCheckpoints(CurrentPipeIndex - 1, CurrentPipeIndex);
}

void UPipeProgressComponent::UpdateWrongWayState()
{
	MaxPipeProgress = FMath::Max(CurrentPipeProgress, MaxPipeProgress);

	// If not known to be going backwards, but made un-progress, we're going backwards
	if (!bWrongWay && CurrentPipeProgress < MaxPipeProgress - 0.33f)
	{
		bWrongWay = true;
	}

	// Going backwards, store how far back we went
	if (bWrongWay)
	{
		MinPipeProgress = FMath::Min(CurrentPipeProgress, MinPipeProgress);

		// If we're forwards again, clear the state
		if (CurrentPipeProgress > MinPipeProgress + 0.33f)
		{
			MinPipeProgress = TNumericLimits<float>::Max();
			MaxPipeProgress = 0.f;

			bWrongWay = false;
		}
	}
}