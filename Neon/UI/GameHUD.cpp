// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "GameHUD.h"

void UGameHUD::SetProgress(int32 PlayerIdx, float ProgressPercent)
{
	
	switch (PlayerIdx)
	{
		case 0: SetRedProgress(ProgressPercent); break;
		case 1: SetGreenProgress(ProgressPercent); break;
		case 2: SetOrangeProgress(ProgressPercent); break;
		case 3: SetBlueProgress(ProgressPercent); break;

		default: UE_LOG(LogNeon, Error, TEXT("Unexpected player index %i!"), PlayerIdx); break;
	}
}
