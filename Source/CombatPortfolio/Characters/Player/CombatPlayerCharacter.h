// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UCombatComponent;
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

UCLASS()
class COMBATPORTFOLIO_API ACombatPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime ) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatComponent> CombatComponent;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> WalkAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ToggleRotationModeAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttackAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DodgeAction;
	
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
	
	FVector2D LastMovementInputVector = FVector2D::ZeroVector;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State", meta = (AllowPrivateAccess = "true"))
	ECombatMovementState MovementState = ECombatMovementState::Running;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State", meta = (AllowPrivateAccess = "true"))
	bool bWantsToWalk = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State", meta = (AllowPrivateAccess = "true"))
	bool bWantsToSprint = false;
	
private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	ECombatRotationMode RotationMode = ECombatRotationMode::OrientToMovement;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float LookSensitivityX = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float LookSensitivityY = 1.0f;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowMovementDebug = true;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void StartWalk();
	void StopWalk();
	
	void StartSprint();
	void StopSprint();
	
	void ToggleRotationMode();
	void SetRotationMode(ECombatRotationMode NewRotationMode);
	void ApplyRotationMode();
	
	void UpdateMovementState();
	void UpdateMovementSpeed();
	
	void Attack();
	void Dodge();
	
	bool IsCombatAttacking() const;
	bool IsCombatDodging() const;
	
	UFUNCTION()
	void HandleCombatActionStateChanged();
	
	FVector GetDodgeDirection() const;
	
	FString GetCombatStateDebugString() const;
	
	FString GetHitWindowDebugString() const;
	
	FString GetComboDebugString() const;
	
	FString GetInvincibilityDebugString() const;
	
	int32 GetHitActorCountDebug() const;
	
	void PrintMovementDebug() const;
};
