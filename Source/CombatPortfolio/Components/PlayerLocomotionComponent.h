// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerLocomotionComponent.generated.h"

class UCombatComponent;
class UStaminaComponent;
class ULockOnComponent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class ECombatMovementState : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting"),
};

UENUM(BlueprintType)
enum class ECombatRotationMode : uint8
{
	OrientToMovement UMETA(DisplayName = "Orient To Movement"),
	Strafe UMETA(DisplayName = "Strafe")
};

UENUM(BlueprintType)
enum class EPlayerCombatMovementCleanupMode : uint8
{
	StopSprintOnly,
	StopMoveInput,
	StopMovementImmediately
};

UCLASS( ClassGroup=(Movement), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UPlayerLocomotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerLocomotionComponent();

protected:
	virtual void BeginPlay() override;

public:
	void Move(const FInputActionValue& Value);
	void StopMove();
	void Look(const FInputActionValue& Value);
	
	void StartWalk();
	void StopWalk();
	
	void StartSprint();
	void StopSprint(bool bShouldUpdateMovement = true);
	
	void HandleCombatActionStarted(EPlayerCombatMovementCleanupMode CleanupMode);
	
	void ToggleRotationMode();
	void SetRotationMode(ECombatRotationMode NewRotationMode);
	void ApplyRotationMode();
	
	void UpdateMovementState();
	void UpdateMovementSpeed();
	void UpdateLockOnRotation(float DeltaSeconds);
	
	FVector GetAttackDirection() const;
	FVector GetDodgeDirection() const;
	void FaceDirection(const FVector& Direction);
	void StopMovementImmediately();
	
	bool IsLockedOn() const;
	
	FString GetMovementStateDebugString() const;
	FString GetRotationModeDebugString() const;
	
private:
	void CacheOwnerComponents();
	
	bool IsMovementBlockedByCombat() const;
	bool IsSprintBlockedByCombat() const;
	
	FVector GetLockOnAttackDirection() const;
	FVector GetMovementInputAttackDirection() const;
	FVector GetCameraForwardAttackDirection() const;
	
	FVector GetFreeDodgeDirection() const;
	FVector GetLockOnDodgeDirection() const;
	FVector GetPlanarDirectionToLockOnTarget() const;
	
	ACharacter* GetOwnerCharacter() const;
	
private:
	UPROPERTY()
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY()
	TObjectPtr<UStaminaComponent> StaminaComponent;
	
	UPROPERTY()
	TObjectPtr<ULockOnComponent> LockOnComponent;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 250.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true"))
	float RunSpeed = 500.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 700.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true"))
	float AttackMoveSpeed = 150.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true"))
	float DodgeMoveSpeed = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ParryMoveSpeed = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Input", meta = (AllowPrivateAccess = "true"))
	float LookSensitivityX = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Input", meta = (AllowPrivateAccess = "true"))
	float LookSensitivityY = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float SprintStaminaDrainRate = 18.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinStaminaToStartSprint = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|LockOn", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float LockOnRotationInterpSpeed = 12.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|Input", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	FVector2D LastMovementInputVector = FVector2D::ZeroVector;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State", meta = (AllowPrivateAccess = "true"))
	ECombatMovementState MovementState = ECombatMovementState::Running;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State", meta = (AllowPrivateAccess = "true"))
	bool bWantsToWalk = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State", meta = (AllowPrivateAccess = "true"))
	bool bWantsToSprint = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|Rotation", meta = (AllowPrivateAccess = "true"))
	ECombatRotationMode RotationMode = ECombatRotationMode::OrientToMovement;
};
