// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "Engine/GameInstance.h"
#include "NeonGameInstance.generated.h"

UCLASS()
class NEON_API UNeonGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UNeonGameInstance();
	
	void Init() override;
	void Shutdown() override;

	///////////////////////// Getters ///////////////////////// 
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Instance, DisplayName = "Get Neon Game Instance", meta = (WorldContext = "WorldContextObject"))
	static UNeonGameInstance *Get(UObject const *WorldContextObject);
		
	///////////////////////// Variables /////////////////////////
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Players)
	int32 NumPlayers;
	
	UPROPERTY(VisibleAnywhere, Category = Players)
	int32 RandomSeed;

	UPROPERTY(BlueprintReadOnly, Category = Highscores)
	class UNeonSaveGame *SaveGame;
};
