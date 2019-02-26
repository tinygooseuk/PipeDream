// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "MiscFunctionLibrary.h"

FText UMiscFunctionLibrary::FormatTH(int32 Number)
{
	// All teens are "th"!
	if (Number > 10 && Number < 20)
	{
		return FText::FromString(TEXT("th"));
	}

	// Else, it depends on the last digit
	int32 LastDigit = Number % 10;
	switch (LastDigit)
	{
		case 1: return FText::FromString(TEXT("st"));
		case 2: return FText::FromString(TEXT("nd"));
		case 3: return FText::FromString(TEXT("rd"));

		default: return FText::FromString(TEXT("th"));
	}
}
