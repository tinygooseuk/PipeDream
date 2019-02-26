// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "Blueprint/UserWidget.h"
#include "GetReady.generated.h"

UCLASS()
class NEON_API UGetReady : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ChangeToGo();	
};
