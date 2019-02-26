// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "PipeSystem.h"

// Engine
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodySetup.h"
#include "TimerManager.h"

// Game
#include "RandomPipeParameters.h"
#include "Components/PipeProgressComponent.h"
#include "NeonGameInstance.h"

DEFINE_LOG_CATEGORY(LogPipeSystem)

APipeSystem::APipeSystem()
	: PipeLength(100)
	, PipeRadius(5000)
	, PipeScale(2.5f)
	, StartSpeed(2000)
	, EndSpeed(6000)
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent *SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);
}

void APipeSystem::BeginPlay()
{
	Super::BeginPlay();
	
	// Generate pipes
	double StartSecs = FPlatformTime::Seconds();

	// Use a seeded random!
	UNeonGameInstance *Instance = UNeonGameInstance::Get(this);
	GOOSE_BAIL(Instance);

	FRandomStream Random;
	Random.Initialize(Instance->RandomSeed);

	// Randomise any zero parameters 
	RandomiseParameters(Random);

	// Generate the pipes (donkey-work!)
	GeneratePipes(Random);

	// Add in "onstacles"
	AddObstacles(Random);

	float ElapsedMS = (float)((FPlatformTime::Seconds() - StartSecs) * 1000.0);
	UE_LOG(LogPipeSystem, Warning, TEXT("Generated %i pipe pieces in %.0fms"), PipePieces.Num(), ElapsedMS);
}

void APipeSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int32 APipeSystem::GetPipeLength() const
{
	return PipeLength;
}

float APipeSystem::GetStartSpeed() const
{
	return StartSpeed;
}
float APipeSystem::GetEndSpeed() const
{
	return EndSpeed;
}

FVector APipeSystem::GetCheckpointLocation(int32 CheckpointIdx)
{
	// No assert
	if (CheckpointIdx < 0 || CheckpointIdx >= PipeLength)
	{
		return FVector::ZeroVector;
	}

	return Checkpoints[CheckpointIdx]->GetComponentLocation();
}

float APipeSystem::EstimateDifficultyOfCurrentSeed()
{
	return 0.1f;
}

void APipeSystem::RandomiseParameters(FRandomStream const &Random)
{
	// Randomise some pipe params, consuming numbers from our stream
	FRandomPipeParameters PipeParams(Random);

	if (PipeLength == 0)
	{
		PipeLength = PipeParams.PipeLength;
		UE_LOG(LogPipeSystem, Display, TEXT("Random length: %i"), PipeParams.PipeLength);
	}

	if (StartSpeed == 0)
	{
		StartSpeed = PipeParams.StartSpeed;
		UE_LOG(LogPipeSystem, Display, TEXT("Random start speed: %i"), PipeParams.StartSpeed);
	}

	if (EndSpeed == 0)
	{
		EndSpeed = PipeParams.EndSpeed;
		UE_LOG(LogPipeSystem, Display, TEXT("Random end speed: %i"), PipeParams.EndSpeed);
	}

	if (PipeScale == 0.f)
	{
		PipeScale = PipeParams.PipeScale;
		UE_LOG(LogPipeSystem, Display, TEXT("Random pipe scale factor: %.2f"), PipeParams.PipeScale);
	}

	if (ObstacleFrequency == 0)
	{
		ObstacleFrequency = PipeParams.ObstacleFrequency;
		UE_LOG(LogPipeSystem, Display, TEXT("Random obstacle frequency: %i"), PipeParams.ObstacleFrequency);
	}
	
	UE_LOG(LogPipeSystem, Display, TEXT("Supposed difficulty: %.2f"), PipeParams.EstimateDifficulty());
}

void APipeSystem::GeneratePipes(FRandomStream const &Random)
{
	SetActorScale3D(FVector(PipeScale));

	GOOSE_BAILF(AvailablePipePieces.Num() > 0, TEXT("Must have >0 pipe pieces to place!"));
	GOOSE_BAILF(AvailablePipePieces.Num() == AvailableTestPipePieces.Num(), TEXT("Must have test pipe pieces matching each pipe piece! (sorry!)"));
	GOOSE_BAILF(AvailablePipeColours.Num() > 0, TEXT("Must have > 0 pipe colour materials to use"));

	FTransform CurrentTransform = GetActorTransform();

	UWorld *World = GetWorld();
	GOOSE_BAIL(World);

	uint32 PipesToRewind = 0;
	uint32 NumFailures = 0;
	for (int32 i = 0; i < PipeLength && NumFailures < 10; i++)
	{
		// Work out piece & colour
		int32 PipeMeshIdx = Random.RandRange(0, AvailablePipePieces.Num() - 1);
		if (i == 0) // Edge-case - first pipe should be the straight one!
		{
			PipeMeshIdx = 0;
		}

		UStaticMesh *TestPipeMesh = AvailableTestPipePieces[PipeMeshIdx];
		UStaticMesh *PipeMesh = AvailablePipePieces[PipeMeshIdx];
		UMaterialInterface *PipeMaterial = AvailablePipeColours[i % AvailablePipeColours.Num()];

		// Set mesh
		UStaticMeshComponent *PipePiece = NewObject<UStaticMeshComponent>(this);
		PipePiece->SetMobility(EComponentMobility::Movable);
		PipePiece->SetStaticMesh(TestPipeMesh);
		PipePiece->SetMaterial(0, PipeMaterial);
		PipePiece->SetGenerateOverlapEvents(true);
		PipePiece->SetCollisionProfileName(TEXT("PipeOverlapElseBlock"));

		PipePiece->RegisterComponent();
		PipePiece->AttachToComponent(GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));

		// Wiggle over to the correct start position
		FVector BeginOffset = PipePiece->GetSocketLocation(TEXT("Begin"));

		// Rotate the pipe	
		int constexpr NUM_SIDES = 8;
		float constexpr ANGLE_PER_SIDE = 360.f / NUM_SIDES;
		float RotationAmount = Random.RandRange(0, NUM_SIDES) * ANGLE_PER_SIDE;

		PipePiece->SetWorldTransform(CurrentTransform);
		PipePiece->AddLocalRotation(FRotator(RotationAmount, 0.f, 0.f));
		PipePiece->AddLocalOffset(-BeginOffset);

		// Check we don't overlap!
		FTransform ComponentTransform = PipePiece->GetComponentTransform();

		TArray<FOverlapResult> Overlaps;
		bool bFailedToPlace = World->ComponentOverlapMulti(Overlaps, PipePiece, ComponentTransform.GetTranslation(), ComponentTransform.GetRotation());

		// Use the final mesh
		PipePiece->SetStaticMesh(PipeMesh);

		// Add the new pipe into the list
		PipePieces.Add(PipePiece);

		// See if we succeeded
		if (bFailedToPlace)
		{
			PipesToRewind++;

			// Rewind some number of pipes
			for (uint32 Rewind = 0; Rewind < PipesToRewind && PipePieces.Num(); Rewind++)
			{
				// Remove last component
				UStaticMeshComponent *PipePiece = PipePieces.Pop();
				PipePiece->DestroyComponent();

				i--;
			}
		}

		// Grab the new end transform - grab the final pipe in case we removed any!
		if (PipePieces.Num() == 0)
		{
			UE_LOG(LogPipeSystem, Error, TEXT("Totally failed to place pipes - starting from scratch!!!"));

			CurrentTransform = GetActorTransform();
			PipesToRewind = 0;

			NumFailures += 1;
		}
		else
		{
			CurrentTransform = PipePieces[PipePieces.Num() - 1]->GetSocketTransform(TEXT("End"));
		}
	}

	if (PipesToRewind > 0)
	{
		UE_LOG(LogPipeSystem, Warning, TEXT("Found and fixed %u errors while generating level"), PipesToRewind);
	}

	if (NumFailures > 0 && PipePieces.Num() == 0)
	{
		UE_LOG(LogPipeSystem, Error, TEXT("Failed to generate any pipe AT ALL after %u tries!!!!"), NumFailures);
	}

	// Create colliders for each of the checkpoints
	for (UStaticMeshComponent *ExistingPipePiece : PipePieces)
	{
		UBoxComponent *Checkpoint = NewObject<UBoxComponent>(this);
		Checkpoint->InitBoxExtent(FVector(PipeRadius * PipeScale, 100.f, PipeRadius * PipeScale));
		Checkpoint->SetCollisionProfileName(TEXT("OverlapAll"));
		Checkpoint->SetMobility(EComponentMobility::Movable);
		Checkpoint->SetWorldLocation(ExistingPipePiece->GetSocketLocation(TEXT("End")));
		Checkpoint->SetWorldRotation(ExistingPipePiece->GetSocketRotation(TEXT("End")));
		Checkpoint->SetGenerateOverlapEvents(true);
		Checkpoint->OnComponentBeginOverlap.AddDynamic(this, &APipeSystem::OnCheckpointOverlap);

		Checkpoint->RegisterComponent();
		Checkpoint->AttachToComponent(GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));

		Checkpoints.Add(Checkpoint);
	}
}

void APipeSystem::AddObstacles(struct FRandomStream const &Random)
{
	static TArray<FString> OBSTACLE_NAMES = { TEXT("Hole"), TEXT("Plus") };
	
	int32 ObstacleFreq = Random.RandRange(ObstacleFrequency, FRandomPipeParameters::ObstacleFrequencyRange.Max);

	for (int32 ObstacleIdx = ObstacleFreq; ObstacleIdx < PipeLength; ObstacleIdx += ObstacleFrequency)
	{
		FTransform ObstacleTransform = PipePieces[ObstacleIdx]->GetSocketTransform(TEXT("Begin"));

		FString ObstacleName = OBSTACLE_NAMES[Random.RandRange(0, OBSTACLE_NAMES.Num()-1)];
		FString ObstacleClassName = FString::Printf(TEXT("Obstacle%s_BP"), *ObstacleName);

		UClass *ObstacleClass = UGooseUtil::GetClass(*ObstacleClassName, TEXT("Obstacles"));
		GOOSE_BAIL_CONTINUE(ObstacleClass);

		UWorld *World = GetWorld();
		GOOSE_BAIL_CONTINUE(World);

		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor *Obstacle = World->SpawnActor(ObstacleClass, &ObstacleTransform, ActorSpawnParameters);
		GOOSE_BAIL_CONTINUE(Obstacle);
		Obstacle->SetActorScale3D(FVector(PipeScale));

		// Compute next frequency
		ObstacleFreq = Random.RandRange(ObstacleFrequency, FRandomPipeParameters::ObstacleFrequencyRange.Max);
	}	
}

void APipeSystem::OnCheckpointOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult const &SweepResult)
{
	UBoxComponent *Checkpoint = Cast<UBoxComponent>(OverlappedComponent);
	GOOSE_BAIL(Checkpoint);

    if (!OtherActor)
    {
        return;
    }
    
    UPipeProgressComponent *PipeProgressComp = OtherActor->FindComponentByClass<UPipeProgressComponent>();
    if (!PipeProgressComp)
    {
        return;
    }
    
    int32 PipeNum = Checkpoints.Find(Checkpoint) + 1; // 1-based please!

    // Update progress on the component
    PipeProgressComp->UpdateProgress(PipeNum, PipeLength, this);
}
