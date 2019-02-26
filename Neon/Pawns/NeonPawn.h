// (c) 2017 TinyGoose Ltd., All Rights Reserved.
#pragma once

#include "Neon.h"
#include "GameFramework/Pawn.h"
#include "NeonPawn.generated.h"

UCLASS(Config=Game)
class ANeonPawn : public APawn
{
	GENERATED_BODY()

public:
	ANeonPawn();
	void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type Reason) override;

	void Tick(float DeltaSeconds) override;
	void NotifyHit(UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	void PossessedBy(AController *NewController) override;
	void UnPossessed() override;

	float TakeDamage(float Damage, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser) override;

	///////////////////////// Component Getters /////////////////////////
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return ShipMesh; }
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE class UPlayerHUD *GetPlayerHUD() const { return PlayerHUD; }

	///////////////////////// Getters /////////////////////////
	UFUNCTION(BlueprintCallable, Category = Health)
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, Category = Health)
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, Category = Movement)
	float GetThrustPercent() const;

	///////////////////////// Death /////////////////////////
	void Die();
	void RespawnFromCheckpoint();

protected:	
	///////////////////////// Input /////////////////////////
	void SetupPlayerInputComponent(class UInputComponent *InputComponent) override; 
	UFUNCTION() void ActionPressed();

    ///////////////////////// Component Delegates /////////////////////////
	UFUNCTION() void OnPipeProgressUpdated(class APipeSystem *PipeSystem);
	void UpdatePipeProgressUI();

    ///////////////////////// Flying /////////////////////////
    void TickMovement(float DeltaSeconds);
    
    ///////////////////////// Health Regen. /////////////////////////
	void TickHealthRegeneration(float DeltaSeconds);

	///////////////////////// Inputs /////////////////////////
	UFUNCTION() void ThrustInput(float Val);
	UFUNCTION() void MoveUpInput(float Val);
	UFUNCTION() void MoveRightInput(float Val);
	UFUNCTION() void RollInput(float Val);
	UFUNCTION() void RequestPause();

	///////////////////////// Speed Handling /////////////////////////
	void UpdateBaseSpeed(class APipeSystem *PipeSystem, float ProgressPercent);

	///////////////////////// Components /////////////////////////
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent *ShipMesh;
	
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent *SpringArm;

	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent *Camera;

	UPROPERTY(Category = Effects, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UParticleSystemComponent *ExplosionParticleSystem;

    UPROPERTY(Category = Gameplay, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UPipeProgressComponent *ProgressComponent;
    
	///////////////////////// HUD/UI /////////////////////////
	UPROPERTY() class UPlayerHUD  *PlayerHUD = nullptr;
	UPROPERTY() class UUserWidget *GameOverUI = nullptr;
	UPROPERTY() class UUserWidget *LevelCompleteUI = nullptr;
	UPROPERTY() class UUserWidget *WrongWayUI = nullptr;
	UPROPERTY() class UGetReady   *GetReadyUI = nullptr;

private:
	void CreateUI();
	void UpdateThrustMaterial(float InThrustAmount);

	void UpdateWrongWayUI();
	
	///////////////////////// Handling /////////////////////////
	UPROPERTY(Category = Ship, EditAnywhere)
	float Acceleration;

	UPROPERTY(Category = Ship, EditAnywhere)
	float TurnSpeed;

	UPROPERTY(Category = Ship, EditAnywhere)
	float RollSpeed;

	UPROPERTY(Category = Ship, EditAnywhere)
	float BaseSpeed;

	UPROPERTY(Category = Ship, EditAnywhere)
	float MaxThrust;

	///////////////////////// Health /////////////////////////
	UPROPERTY(Category = Health, EditAnywhere)
	float InitialHealth;

	float Health;
	float HealthRegenerationTimer;

	///////////////////////// Movement /////////////////////////
	float CurrentForwardSpeed;
	float CurrentYawSpeed;
	float CurrentPitchSpeed;
	float CurrentRollSpeed;

	///////////////////////// State/Misc /////////////////////////
	int32 PlayerIdx;
	int32 bPossessed : 1;
	int32 bActivated : 1;
	int32 bDead : 1;
	int32 bCompletedLevel : 1;
	int32 bRemoveGameOverUI : 1;

	// The transform we should use when respawning from the last checkpoint
	FTransform LastCheckpointTransform;

	FTimerHandle ShowGameOverTimerHandle;

	///////////////////////// Materials ////////////////////
	UPROPERTY() class UMaterialInstanceDynamic *ThrustMaterial = nullptr;
};
