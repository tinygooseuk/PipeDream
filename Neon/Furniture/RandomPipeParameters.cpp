// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "RandomPipeParameters.h"

TRandomPipeParameterRange<int32> FRandomPipeParameters::PipeLengthRange(20, 60);
TRandomPipeParameterRange<int32> FRandomPipeParameters::StartSpeedRange(1500, 4000);
TRandomPipeParameterRange<int32> FRandomPipeParameters::EndSpeedRange(8000, 15000);
TRandomPipeParameterRange<float> FRandomPipeParameters::PipeScaleRange(1.5f, 4.f, true);		// Inverted - small is harder!
TRandomPipeParameterRange<int32> FRandomPipeParameters::ObstacleFrequencyRange(3, 10, true);	// Inverted - small is harder!

FRandomPipeParameters::FRandomPipeParameters(FRandomStream const &Random)
{
	RandomiseParameters(Random);	
}

void FRandomPipeParameters::RandomiseParameters(FRandomStream const &Random)
{
	GOOSE_CHECKF(Random.GetInitialSeed() == Random.GetCurrentSeed(), TEXT("Getting random params from a non-rewound random stream!"));

	PipeLength = PipeLengthRange.Get(Random);
	StartSpeed = StartSpeedRange.Get(Random);
	EndSpeed = EndSpeedRange.Get(Random);
	PipeScale = PipeScaleRange.Get(Random);
	ObstacleFrequency = ObstacleFrequencyRange.Get(Random);
}

float FRandomPipeParameters::EstimateDifficulty()
{
	float AvgDifficulty = 0.f;
	float MaxDifficulty = 0.f;

	int32 NumDifficulties = 5;

	AvgDifficulty += PipeLengthRange.GetDifficulty(PipeLength);
	AvgDifficulty += StartSpeedRange.GetDifficulty(StartSpeed);
	AvgDifficulty += EndSpeedRange.GetDifficulty(EndSpeed);
	AvgDifficulty += PipeScaleRange.GetDifficulty(PipeScale);
	AvgDifficulty += ObstacleFrequencyRange.GetDifficulty(ObstacleFrequency);

	AvgDifficulty /= float(NumDifficulties);

	MaxDifficulty = FMath::Max(MaxDifficulty, PipeLengthRange.GetDifficulty(PipeLength));
	MaxDifficulty = FMath::Max(MaxDifficulty, StartSpeedRange.GetDifficulty(StartSpeed));
	MaxDifficulty = FMath::Max(MaxDifficulty, EndSpeedRange.GetDifficulty(EndSpeed));
	MaxDifficulty = FMath::Max(MaxDifficulty, PipeScaleRange.GetDifficulty(PipeScale));
	MaxDifficulty = FMath::Max(MaxDifficulty, ObstacleFrequencyRange.GetDifficulty(ObstacleFrequency));

	UE_LOG(LogNeon, Warning, TEXT("Avg. difficulty: %.2f, Max. difficulty: %.2f"), AvgDifficulty, MaxDifficulty);

	return AvgDifficulty;
}