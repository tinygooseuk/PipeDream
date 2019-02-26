// Deliberately no pragma once!

// Engine
#include "Math/RandomStream.h"

template <typename T>
inline float TRandomPipeParameterRange<T>::GetDifficulty(T Input)
{
	float Range = float(Max - Min);
	float FloatInput = float(Input - Min);

	float LerpValue = FloatInput / Range;
	return bInverted ? 1.f - LerpValue : LerpValue;
}

template <typename T>
inline T TRandomPipeParameterRange<T>::Get(FRandomStream const &Random)
{
	return T(); // Return the default T :(
}

// Concrete implementation for ints
template <>
inline int32 TRandomPipeParameterRange<int32>::Get(FRandomStream const &Random)
{
	return Random.RandRange(Min, Max);
}

// Concrete implementation for floats
template <>
inline float TRandomPipeParameterRange<float>::Get(FRandomStream const &Random)
{
	return Random.FRandRange(Min, Max);
}