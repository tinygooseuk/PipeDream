// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MiscFunctionLibrary.generated.h"

UCLASS()
class NEON_API UMiscFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Formatting)
	static FText FormatTH(int32 Number);
};
