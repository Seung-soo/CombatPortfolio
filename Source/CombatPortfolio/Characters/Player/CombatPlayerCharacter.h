#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UCombatComponent;
class UStaminaComponent;
class UHealthComponent;
class ULockOnComponent;
class UHitStopComponent;
class UPlayerAttackComponent;
class UPlayerDefenseComponent;
class UPlayerReactionComponent;
class UPlayerHUDComponent;
class UPlayerLocomotionComponent;
struct FInputActionValue;

UCLASS()
class COMBATPORTFOLIO_API ACombatPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACombatPlayerCharacter();
	
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DebugApplyDamageToPlayer(float DamageAmount);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime ) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerAttackComponent> PlayerAttackComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerDefenseComponent> PlayerDefenseComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerReactionComponent> PlayerReactionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaminaComponent> StaminaComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULockOnComponent> LockOnComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHitStopComponent> HitStopComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerHUDComponent> PlayerHUDComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerLocomotionComponent> PlayerLocomotionComponent;

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
	TObjectPtr<UInputAction> LightAttackAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> HeavyAttackAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DodgeAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LockOnAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ParryAction;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowMovementDebug = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawDodgeDirectionDebug = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DodgeDirectionDebugLength = 250.0f;

private:
	void Move(const FInputActionValue& Value);
	void StopMove();
	void Look(const FInputActionValue& Value);
	
	void StartWalk();
	void StopWalk();
	
	void StartSprint();
	void StopSprint();
	
	void ToggleRotationMode();
	
	void Attack();
	void Dodge();
	void HeavyAttack();
	void Parry();

	void ToggleLockOn();
	
	void UpdateCharacterTickEnabled();
	
	UFUNCTION()
	void HandleCombatActionStateChanged();
	
	UFUNCTION()
	void HandleStaminaDepleted();

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta);
	
	UFUNCTION()
	void HandleDamaged(const FCombatDamageInfo& DamageInfo);
	
	UFUNCTION()
	void HandleDeath();
	
	UFUNCTION()
	void HandleLockOnTargetChanged();
	
	FString GetStaminaDebugString() const;
	
	FString GetHealthDebugString() const;
	
	FString GetCombatStateDebugString() const;
	
	FString GetHitWindowDebugString() const;
	
	FString GetComboDebugString() const;
	
	FString GetInvincibilityDebugString() const;
	
	FString GetLockOnDebugString() const;
	
	int32 GetHitActorCountDebug() const;
	
	void PrintMovementDebug() const;
	
	void DrawDodgeDirectionDebug(const FVector& DodgeDirection) const;
};
