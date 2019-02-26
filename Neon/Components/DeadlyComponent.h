// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeadlyComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEON_API UDeadlyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeadlyComponent();

	UFUNCTION(BlueprintCallable, Category = Damage)
	FORCEINLINE float GetDamageDealt() const { return DamageDealt; }

private:
	UPROPERTY(EditAnywhere)
	float DamageDealt;
};
