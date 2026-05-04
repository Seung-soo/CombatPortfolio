// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthComponent.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "CombatComponent.generated.h"

class UAnimInstance;
class UAnimMontage;
class UHealthComponent;

UENUM(BlueprintType)
enum class ECombatActionState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Attacking UMETA(DisplayName = "Attacking"),
	Dodging UMETA(DisplayName = "Dodging"),
};

USTRUCT(BlueprintType)
struct FComboAttackData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FName SectionName = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0"))
	float Damage = 25.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "1.0"))
	float TraceRadius = 80.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0"))
	float TraceForwardOffset = 140.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0"))
	float TraceHalfHeight = 40.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatActionStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitWindowChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvincibilityChangedSignature);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class COMBATPORTFOLIO_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	bool RequestAttack();
	bool RequestDodge(const FVector& DodgeDirection);
	
	bool CanStartAttack() const;
	bool CanStartDodge() const;
	
	bool IsAttacking() const;
	bool IsDodging() const;
	bool IsInvincible() const;
	bool IsHitWindowOpen() const;
	bool IsComboInputWindowOpen() const;
	bool HasBufferedComboInput() const;
	int GetCurrentComboIndex() const;
	int32 GetHitActorCountThisAttack() const;
	float GetCurrentAttackDamage() const;
	ECombatActionState GetCombatActionState() const;
	
	void BeginHitWindow();
	void EndHitWindow();
	
	void BeginComboInputWindow();
	void EndComboInputWindow();
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnCombatActionStateChangedSignature OnCombatActionStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnHitWindowChangedSignature OnHitWindowChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnComboStateChangedSignature OnComboStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnInvincibilityChangedSignature OnInvincibilityChanged;
	
private:
	bool StartAttack();
	bool StartDodge(const FVector& DodgeDirection);
	
	bool TryPlayDodgeMontage();
	void ApplyDodgeMovement(const FVector& DodgeDirection);
	
	void BeginInvincibility();
	void EndInvincibility();
	
	bool TryBufferComboInput();
	bool TryCommitBufferedCombo();
	bool CanMoveToNextCombo() const;
	const FComboAttackData* GetCurrentComboAttackData() const;
	const FComboAttackData* GetComboAttackDataByIndex(int32 ComboIndex) const;
	FName GetCurrentComboSectionName() const;
	FName GetNextComboSectionName() const;
	void ResetComboState();
	
	UAnimInstance* GetOwnerAnimInstance() const;
	
	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void HandleDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void FinishAttack();
	void FinishDodge();
	
	void SetCombatActionState(ECombatActionState NewCombatActionState);
	
	void SetHitWindowOpen(bool bNewHitWindowOpen);
	
	void SetComboInputWindowOpen(bool bNewComboInputWindowOpen);
	void SetComboInputBuffered(bool bNewComboInputBuffered);
	
	void SetInvincible(bool bNewInvincible);
	
	void PerformAttackTrace();
	void ApplyDamageToHitActor(AActor* HitActor);
	bool HasAlreadyHitActor(const AActor* HitActor) const;
	void RegisterHitActor(AActor* HitActor);
	FVector GetAttackTraceStartLocation() const;
	FVector GetAttackTraceEndLocation() const;
	
	bool IsDamageBlockedByInvincibility(const AActor* TargetActor) const;

	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	float AttackPlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Dodge", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DodgeMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Dodge", meta = (AllowPrivateAccess = "true"))
	float DodgePlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Dodge", meta = (AllowPrivateAccess = "true"))
	float DodgeStrength = 900.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Dodge", meta = (AllowPrivateAccess = "true"))
	float DodgeDuration = 0.45f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Dodge", meta = (AllowPrivateAccess = "true"))
	float DodgeInvincibleDuration = 0.25f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	TArray<FComboAttackData> ComboAttackDataList;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawAttackTraceDebug = true;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	ECombatActionState CombatActionState = ECombatActionState::Idle;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	bool bHitWindowOpen = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	bool bInvincible = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	int32 CurrentComboIndex = 0;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	bool bComboInputWindowOpen = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	bool bComboInputBuffered = false;
	
private:
	FTimerHandle DodgeFallbackTimerHandle;
	FTimerHandle InvincibilityTimerHandle;
	
	TArray<TWeakObjectPtr<AActor>> HitActorsThisAttack;
};
