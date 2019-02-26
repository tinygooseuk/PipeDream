// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "Neon.h"
#include "GameFramework/Actor.h"
#include "PipeSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPipeSystem, Log, All);

UCLASS()
class NEON_API APipeSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	APipeSystem();

	void BeginPlay() override;
	void Tick(float DeltaTime) override;

	///////////////////////// Getters /////////////////////////
	UFUNCTION(BlueprintCallable, Category = Pipes)
	int32 GetPipeLength() const;

	UFUNCTION(BlueprintCallable, Category = Pipes)
	float GetStartSpeed() const;
	UFUNCTION(BlueprintCallable, Category = Pipes)
	float GetEndSpeed() const;

	UFUNCTION(BlueprintCallable, Category = Pipes)
	FVector GetCheckpointLocation(int32 CheckpointIdx);

	///////////////////////// Static Functions /////////////////////////
	static float EstimateDifficultyOfCurrentSeed();

protected:	
	void RandomiseParameters(struct FRandomStream const &Random);
	void GeneratePipes(struct FRandomStream const &Random);
	void AddObstacles(struct FRandomStream const &Random);	
	
	UFUNCTION()
	void OnCheckpointOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult const &SweepResult);

	TArray<class UStaticMeshComponent *> PipePieces;
	TArray<class UBoxComponent *> Checkpoints;

private:
	///////////////////////// Editor Properties /////////////////////////
	UPROPERTY(EditAnywhere, Category = Pipes)
	int32 PipeLength;

	UPROPERTY(EditAnywhere, Category = Pipes)
	int32 PipeRadius;

	UPROPERTY(EditAnywhere, Category = Pipes)
	int32 ObstacleFrequency;
	
	UPROPERTY(EditAnywhere, Category = Pipes)
	float PipeScale;

	UPROPERTY(EditAnywhere, Category = Ships)
	float StartSpeed;

	UPROPERTY(EditAnywhere, Category = Ships)
	float EndSpeed;

	UPROPERTY(EditAnywhere, Category = Pipes)
	TArray<class UStaticMesh *> AvailableTestPipePieces;

	UPROPERTY(EditAnywhere, Category = Pipes)
	TArray<class UStaticMesh *> AvailablePipePieces;

	UPROPERTY(EditAnywhere, Category = Pipes)
	TArray<class UMaterialInterface *> AvailablePipeColours;
};
