// (c) 2017 TinyGoose Ltd., All Rights Reserved.

#include "NeonPawn.h"

// Engine
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

// Game
#include "Components/PipeProgressComponent.h"
#include "Components/DeadlyComponent.h"
#include "Furniture/PipeSystem.h"
#include "GameModes/NeonGameModeBase.h"
#include "UI/PlayerHUD.h"
#include "UI/GetReady.h"

static struct FNeonPawnResources 
{
	float const HEALTH_REGENERATION_DELAY = 0.45f;
	float const HEALTH_REGENERATION_SPEED = 40.f;

	int32 const MATERIAL_MAIN_INDEX = 0;
	int32 const MATERIAL_THRUST_INDEX = 1;

	FName const MATERIAL_THRUST_AMOUNT_NAME = TEXT("ThrustAmount");
} NeonPawnResources;

ANeonPawn::ANeonPawn()
	: Acceleration(2000.f)
	, TurnSpeed(70.f)
	, RollSpeed(150.f)
	, BaseSpeed(2000.f)
	, MaxThrust(4000.f)
	, InitialHealth(100.f)
	, Health(0.f)
	, HealthRegenerationTimer(0.f)
	, CurrentForwardSpeed(0.f)
	, CurrentYawSpeed(0.f)
	, CurrentPitchSpeed(0.f)
	, CurrentRollSpeed(0.f)
	, PlayerIdx(-1)
	, bPossessed(false)
	, bActivated(false)
	, bDead(false)
	, bCompletedLevel(false)
	, bRemoveGameOverUI(false)
{
	// Get resources
	static ConstructorHelpers::FObjectFinderOptional<UStaticMesh>		SM_Ship(TEXT("/Game/Meshes/SM_Ship.SM_Ship"));
	static ConstructorHelpers::FObjectFinderOptional<UParticleSystem>	PS_Explosion(TEXT("ParticleSystem'/Game/ParticleSystems/PS_Explosion.PS_Explosion'"));
	
	// Create static mesh component
	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	ShipMesh->SetWorldScale3D(FVector(1.5f));
	ShipMesh->SetStaticMesh(SM_Ship.Get());	// Set static mesh
	SetRootComponent(ShipMesh);

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 240.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f,0.f,60.f);
	SpringArm->bEnableCameraLag = true;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 25.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller

	// Create particle system
	ExplosionParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ExplosionPS"));
	ExplosionParticleSystem->Template = PS_Explosion.Get();
	ExplosionParticleSystem->SetActive(true);
	ExplosionParticleSystem->SetupAttachment(ShipMesh);
}

void ANeonPawn::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = true;
	
	// Create our progress component
	ProgressComponent = NewObject<UPipeProgressComponent>(this, TEXT("Progress Component"));
	ProgressComponent->OnPipeProgressUpdated.BindDynamic(this, &ANeonPawn::OnPipeProgressUpdated);
	ProgressComponent->RegisterComponent();

	// Create dynamic material instance for detailing/thrust material
	UMaterialInterface *BaseMaterial = ShipMesh->GetMaterial(NeonPawnResources.MATERIAL_THRUST_INDEX);
	ThrustMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	ShipMesh->SetMaterial(NeonPawnResources.MATERIAL_THRUST_INDEX, ThrustMaterial);

	// Set initial health & speed
	Health = InitialHealth;
	CurrentForwardSpeed = BaseSpeed;
}

void ANeonPawn::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	
	FTimerManager &TimerManager = GetWorldTimerManager();
	TimerManager.ClearTimer(ShowGameOverTimerHandle);
}

void ANeonPawn::PossessedBy(AController *NewController) 
{
	Super::PossessedBy(NewController);

	bPossessed = true;

	// Work out this player number
	PlayerIdx = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It, ++PlayerIdx)
	{
		if (NewController == It->Get())
		{
			break;
		}
	}

	CreateUI();

	// Set our colour based on index
	UMaterialInstanceConstant *SHIP_MATERIALS[] =
	{
		UGooseUtil::GetObject<UMaterialInstanceConstant>(TEXT("MI_TechGrid_Red"), TEXT("Materials")),
		UGooseUtil::GetObject<UMaterialInstanceConstant>(TEXT("MI_TechGrid_Green"), TEXT("Materials")),
		UGooseUtil::GetObject<UMaterialInstanceConstant>(TEXT("MI_TechGrid_Orange"), TEXT("Materials")),
		UGooseUtil::GetObject<UMaterialInstanceConstant>(TEXT("MI_TechGrid_Blue"), TEXT("Materials"))
	};
	ShipMesh->SetMaterial(NeonPawnResources.MATERIAL_MAIN_INDEX, SHIP_MATERIALS[PlayerIdx]);

	UWorld *World = GetWorld();
	GOOSE_BAIL(World);

	ANeonGameModeBase *BaseGameMode = Cast<ANeonGameModeBase>(World->GetAuthGameMode());
	GOOSE_BAIL(BaseGameMode);

	if (BaseGameMode->IsRealGameMode())
	{
		// Create a "get ready UI"
		APlayerController *PC = Cast<APlayerController>(GetController());
		GOOSE_BAIL(PC);

		UClass *UMG_GetReady = UGooseUtil::GetClass(TEXT("UMG_GetReady"), TEXT("UI/InGame"));
		GOOSE_BAIL(UMG_GetReady);

		GetReadyUI = CreateWidget<UGetReady>(PC, UMG_GetReady);
		GetReadyUI->AddToPlayerScreen(PlayerIdx);
	}
}
void ANeonPawn::UnPossessed() 
{
	Super::UnPossessed();

	bPossessed = false;

	GOOSE_BAIL(PlayerHUD);
	PlayerHUD->RemoveFromParent();
	PlayerHUD = nullptr;
}

void ANeonPawn::Tick(float DeltaSeconds)
{
	if (!IsDead() && bPossessed && !bCompletedLevel)
	{
		TickMovement(DeltaSeconds);
		TickHealthRegeneration(DeltaSeconds);

		UpdatePipeProgressUI();
		UpdateWrongWayUI();
	}

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}


void ANeonPawn::NotifyHit(UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, FHitResult const &Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	FQuat MyQuat = CurrentRotation.Quaternion();
	FQuat SurfaceQuat = HitNormal.ToOrientationQuat();

	SetActorRotation(FQuat::Slerp(MyQuat, SurfaceQuat, 0.05f));

	// Work out crash intensity
	float CrashIntensity = 1.f - (MyQuat.AngularDistance(SurfaceQuat) / (PI * 2)); // 0..1, dependant in angular difference

	APlayerController *PC = Cast<APlayerController>(GetController());
	GOOSE_BAIL(PC);

	// Remove health
	FDamageEvent DamageEvent;
	TakeDamage(CrashIntensity, DamageEvent, nullptr, Other);

	// Remove more health if "deadly"
	if (UDeadlyComponent *DeadlyComponent = Other->FindComponentByClass<UDeadlyComponent>())
	{
		FDamageEvent DeadlyDamageEvent;
		TakeDamage(CrashIntensity * DeadlyComponent->GetDamageDealt(), DeadlyDamageEvent, nullptr, Other);
	}

	// Remove speed
	CurrentForwardSpeed *= CrashIntensity;

	// Vibrate player's controller
	PC->PlayDynamicForceFeedback(CrashIntensity, 0.2f, true, true, true, true, EDynamicForceFeedbackAction::Start);

	// Shake player's screen
	UClass *CrashShake_BP = UGooseUtil::GetClass("CrashShake_BP");
	GOOSE_BAIL(CrashShake_BP);
	PC->ClientPlayCameraShake(CrashShake_BP);
}

float ANeonPawn::TakeDamage(float Damage, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
	Damage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	// Dock health
	Health -= Damage;

	// Bit weird - if health <= 0, die!
	if (IsDead())
	{
		Die();
	}

	// Reset the regeneration timer
	HealthRegenerationTimer = NeonPawnResources.HEALTH_REGENERATION_DELAY;

	return Damage;
}

bool ANeonPawn::IsDead() const
{
	return bDead || Health <= 0.f;
}

float ANeonPawn::GetHealthPercent() const
{
	return Health / InitialHealth;
}


float ANeonPawn::GetThrustPercent() const
{
	return (CurrentForwardSpeed - BaseSpeed) / MaxThrust;
}


void ANeonPawn::Die()
{
	// First of all, tell the game mode
	UWorld *World = GetWorld();
	GOOSE_BAIL(World);
	
	ANeonGameModeBase *GameMode = Cast<ANeonGameModeBase>(World->GetAuthGameMode());
	GOOSE_BAIL(GameMode);
	
	GameMode->PlayerDied(PlayerIdx);

	// Hide mesh
	ShipMesh->SetHiddenInGame(true);

	// Explode!
	ExplosionParticleSystem->ActivateSystem(true);

	// Fade to black
	APlayerCameraManager *MyCameraManager = UGameplayStatics::GetPlayerCameraManager(this, PlayerIdx);
	GOOSE_BAIL(MyCameraManager);
	
	MyCameraManager->StartCameraFade(0.f, 1.f, 2.f, FLinearColor::Black, false, true);

	// Show UI
	FTimerManager &TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(ShowGameOverTimerHandle, [=] 
	{
		if (bRemoveGameOverUI)
		{
			bRemoveGameOverUI = false;
			return;
		}
	
		UWorld *World = GetWorld();
		GOOSE_BAIL(World);

		ANeonGameModeBase *GameMode = Cast<ANeonGameModeBase>(World->GetAuthGameMode());
		GOOSE_BAIL(GameMode);

		GameOverUI = GameMode->CreateGameOverUI(PlayerIdx);
		GOOSE_BAIL(GameOverUI);

		GameOverUI->AddToPlayerScreen();
	}, 2.f, false);

	// Remove any "wrong way" UI
	if (WrongWayUI)
	{
		WrongWayUI->RemoveFromParent();
		WrongWayUI = nullptr;
	}

	bDead = true;
}

void ANeonPawn::RespawnFromCheckpoint()
{
	// Find last checkpoint index
	SetActorTransform(LastCheckpointTransform);

	// Un-hide mesh
	ShipMesh->SetHiddenInGame(false);

	// Fade from black
	APlayerCameraManager *MyCameraManager = UGameplayStatics::GetPlayerCameraManager(this, PlayerIdx);
	GOOSE_BAIL(MyCameraManager);

	MyCameraManager->StartCameraFade(1.f, 0.f, 0.5f, FLinearColor::Black, false, true);

	// Remove game over UI, if any!
	if (GameOverUI)
	{
		GameOverUI->RemoveFromParent();
		GameOverUI = nullptr;
	}
	else
	{
		bRemoveGameOverUI = true;
	}

	Health = InitialHealth;
	bDead = false;
}

void ANeonPawn::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    // Check if PlayerInputComponent is valid (not NULL)
	GOOSE_BAIL(PlayerInputComponent);

	// Bind our control axis' to callback functions
	PlayerInputComponent->BindAxis(TEXT("Thrust"), this, &ANeonPawn::ThrustInput);
	PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &ANeonPawn::MoveUpInput);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ANeonPawn::MoveRightInput);
	PlayerInputComponent->BindAxis(TEXT("Roll"), this, &ANeonPawn::RollInput);

	// Bind our control actions to callback functions
	PlayerInputComponent->BindAction(TEXT("Pause"), EInputEvent::IE_Pressed, this, &ANeonPawn::RequestPause);
	PlayerInputComponent->BindAction(TEXT("Action"), EInputEvent::IE_Pressed, this, &ANeonPawn::ActionPressed);
}

void ANeonPawn::ActionPressed()
{
	if (bDead)
	{
		UWorld *World = GetWorld();
		GOOSE_BAIL(World);

		ANeonGameModeBase *GameMode = Cast<ANeonGameModeBase>(World->GetAuthGameMode());
		GOOSE_BAIL(GameMode);

		if (GameMode->CanPlayerRespawn(PlayerIdx, this))
		{
			UE_LOG(LogNeon, Warning, TEXT("Dead Boop - respawnable!"));
			RespawnFromCheckpoint();
		}
		else
		{
			UE_LOG(LogNeon, Warning, TEXT("Dead Boop - no respawn!"));
		}
	}
	else
	{
		UE_LOG(LogNeon, Warning, TEXT("Boop!"));
	}
}

void ANeonPawn::OnPipeProgressUpdated(APipeSystem *PipeSystem)
{
	if (!bActivated)
	{
		bActivated = true;

		if (GOOSE_VERIFY(GetReadyUI)) 
		{
			GetReadyUI->ChangeToGo();
		}
	}

	// Store the transform of this checkpoint for when dying
	LastCheckpointTransform = GetActorTransform();
	LastCheckpointTransform.SetLocation(PipeSystem->GetCheckpointLocation(ProgressComponent->GetCurrentPipeIndex()));

	GOOSE_BAIL(ProgressComponent);
	
	// Get the progress [0..1] through the entire pipe
	float Progress = ProgressComponent->GetCurrentPipeProgress() / float(PipeSystem->GetPipeLength());

	// Change the speed of the pawn based on the pipe we're in!
	UpdateBaseSpeed(PipeSystem, Progress);

	// Check if we won!
	bool bHasWon = (ProgressComponent->GetCurrentPipeIndex() == PipeSystem->GetPipeLength());
	if (!LevelCompleteUI && bHasWon)
	{
		UWorld *World = GetWorld();															GOOSE_BAIL(World);
		ANeonGameModeBase *GameMode = Cast<ANeonGameModeBase>(World->GetAuthGameMode());	GOOSE_BAIL(GameMode);
	
		// Add the custom UI
		LevelCompleteUI = GameMode->CreateLevelCompleteUI(PlayerIdx);
		GOOSE_BAIL(LevelCompleteUI);

		LevelCompleteUI->AddToPlayerScreen();

		// Deactivate movement etc.
		bCompletedLevel = true;
	}	
}

void ANeonPawn::UpdatePipeProgressUI()
{
	GOOSE_BAIL(ProgressComponent);

	// Get the progress [0..1] through the entire pipe	
	APipeSystem *PipeSystem = ProgressComponent->GetLastSeenPipeSystem();
	if (PipeSystem)
	{
		float Progress = ProgressComponent->GetCurrentPipeProgress() / float(PipeSystem->GetPipeLength());

		// Update the main UI
		ANeonGameModeBase *GameMode = Cast<ANeonGameModeBase>(GetWorld()->GetAuthGameMode());
		GOOSE_BAIL(GameMode);

		GameMode->SetProgressForPlayer(PlayerIdx, Progress);
	}
}


void ANeonPawn::TickMovement(float DeltaSeconds)
{
	const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);

	// Move plan forwards (with sweep so we stop when we collide with things)
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation this frame
	FRotator DeltaRotation(0, 0, 0);
	DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	// Rotate plane
	AddActorLocalRotation(DeltaRotation);	
}

void ANeonPawn::TickHealthRegeneration(float DeltaSeconds)
{
	// Timer is still ticking!
	if (HealthRegenerationTimer > 0.f)
	{
		HealthRegenerationTimer -= DeltaSeconds;
	}
	// We could regenerate?
	else if (Health < InitialHealth)
	{
		// Regenerate a bit, but not too much!
		Health += FMath::Min(InitialHealth, NeonPawnResources.HEALTH_REGENERATION_SPEED * DeltaSeconds);
	}

	// Update HUD
	GOOSE_BAIL(PlayerHUD);
	PlayerHUD->SetHealth(GetHealthPercent());

	ANeonGameModeBase *GameMode = Cast<ANeonGameModeBase>(GetWorld()->GetAuthGameMode());
	GOOSE_BAIL(GameMode);

	GameMode->SetPixellationForPlayer(PlayerIdx, GetHealthPercent());
}

void ANeonPawn::ThrustInput(float Val)
{
	if (!bActivated || bCompletedLevel)
	{
		return;
	}

	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration) : (-0.5f * Acceleration);
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, BaseSpeed, BaseSpeed + MaxThrust);

	// Update the HUD
	float ThrustPercent = GetThrustPercent();

	if (GOOSE_VERIFY(PlayerHUD))
	{
		PlayerHUD->SetSpeed(ThrustPercent);
	}

	UpdateThrustMaterial(ThrustPercent);
}

void ANeonPawn::MoveUpInput(float Val)
{
	if (!bActivated || bCompletedLevel)
	{
		return;
	}

	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	// When steering, we decrease pitch slightly
	TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ANeonPawn::MoveRightInput(float Val)
{
	if (!bActivated || bCompletedLevel)
	{
		return;
	}

	// Target yaw speed is based on input
	float TargetYawSpeed = (Val * TurnSpeed);

	// Smoothly interpolate to target yaw speed
	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ANeonPawn::RollInput(float Val)
{
	if (!bActivated || bCompletedLevel)
	{
		return;
	}

	// Target yaw speed is based on input
	float TargetRollSpeed = (Val * RollSpeed);

	// Smoothly interpolate to target yaw speed
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ANeonPawn::RequestPause()
{
	UWorld *World = GetWorld();
	GOOSE_BAIL(World);

	ANeonGameModeBase *GameMode = Cast<ANeonGameModeBase>(World->GetAuthGameMode());
	GOOSE_BAIL(GameMode);

	GameMode->RequestPause(PlayerIdx);
}

void ANeonPawn::UpdateBaseSpeed(APipeSystem *PipeSystem, float ProgressPercent)
{
	BaseSpeed = FMath::Lerp(PipeSystem->GetStartSpeed(), PipeSystem->GetEndSpeed(), ProgressPercent);
}

void ANeonPawn::CreateUI()
{
	UClass *UMG_PlayerHUD = UGooseUtil::GetClass(TEXT("UMG_PlayerHUD"), TEXT("UI/InGame"));
	GOOSE_BAIL(UMG_PlayerHUD);

	APlayerController *PC = Cast<APlayerController>(GetController());
	GOOSE_BAIL(PC);

	PlayerHUD = CreateWidget<UPlayerHUD>(PC, UMG_PlayerHUD);
	GOOSE_BAIL(PlayerHUD);

	PlayerHUD->AddToPlayerScreen();

	// Set default state!
	PlayerHUD->SetHealth(1.f);
}

void ANeonPawn::UpdateThrustMaterial(float InThrustAmount)
{
	GOOSE_BAIL(ThrustMaterial);
	ThrustMaterial->SetScalarParameterValue(NeonPawnResources.MATERIAL_THRUST_AMOUNT_NAME, InThrustAmount);
}

void ANeonPawn::UpdateWrongWayUI()
{
 	if (IsDead())
	{
		return;
	}

	GOOSE_BAIL(ProgressComponent);

	bool bWrongWay = ProgressComponent->IsGoingTheWrongWay();
	if (bWrongWay && !WrongWayUI)
	{
		// Create wrong way UI
		APlayerController *PC = Cast<APlayerController>(GetController());
		GOOSE_BAIL(PC);

		UClass *UMG_WrongWay = UGooseUtil::GetClass(TEXT("UMG_WrongWay"), TEXT("UI/InGame"));
		GOOSE_BAIL(UMG_WrongWay);

		WrongWayUI = CreateWidget<UUserWidget>(PC, UMG_WrongWay);
		WrongWayUI->AddToPlayerScreen(PlayerIdx);
	}
	else if (!bWrongWay && WrongWayUI)
	{
		// Remove wrong way UI
		WrongWayUI->RemoveFromParent();
		WrongWayUI = nullptr;
	}
}