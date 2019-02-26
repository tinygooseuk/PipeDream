// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"

// Engine
#include "Math/RandomStream.h"

template <typename T> // T must be int32 or float 
struct TRandomPipeParameterRange
{
	TRandomPipeParameterRange(T Min, T Max, bool bInverted = false) 
		: Min(Min), Max(Max), bInverted(bInverted) { }

	T Min;
	T Max;
	bool bInverted;
	
	// Get the difficulty of a given value 
	float GetDifficulty(T Input);

	// Get a value in our range from a random stream
	T Get(FRandomStream const &Random);
};

// Implementation for TRandomPipeParameterRange template class
#include "RandomPipeParameters.imp.h"

struct FRandomPipeParameters
{
	FRandomPipeParameters(FRandomStream const &Random);

	///////////////////////// Functions /////////////////////////
	// Randomise all vars based on their ranges
	void RandomiseParameters(FRandomStream const &Random);
	// Estimate the total difficult of this set of parameters
	float EstimateDifficulty();

	///////////////////////// Variables /////////////////////////
	int32 PipeLength;
	int32 StartSpeed;
	int32 EndSpeed;
	int32 ObstacleFrequency;

	float PipeScale;

	///////////////////////// Ranges /////////////////////////
	static TRandomPipeParameterRange<int32> PipeLengthRange;
	static TRandomPipeParameterRange<int32> StartSpeedRange;
	static TRandomPipeParameterRange<int32> EndSpeedRange;
	static TRandomPipeParameterRange<int32> ObstacleFrequencyRange;
	 
	static TRandomPipeParameterRange<float> PipeScaleRange;
};