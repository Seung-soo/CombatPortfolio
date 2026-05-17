#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "CombatPortfolio/Combat/CombatDamageType.h"
#include "CombatPortfolio/Data/CombatAttackData.h"
#include "CombatComponent.generated.h"

class UAnimInstance;
class UAnimMontage;
class UHealthComponent;
class UStaminaComponent;

UENUM(BlueprintType)
enum class ECombatActionState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Attacking UMETA(DisplayName = "Attacking"),
	Dodging UMETA(DisplayName = "Dodging"),
	HitReaction UMETA(DisplayName = "HitReaction"),
	Dead UMETA(DisplayName = "Dead"),
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
	UCombatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	bool RequestAttack(ECombatAttackInputType AttackInputType);
	bool RequestDodge(const FVector& DodgeDirection);
	bool RequestDeath();
	
	bool CanStartAttack() const;
	bool CanStartDodge() const;
	
	bool IsAttacking() const;
	bool IsDodging() const;
	bool IsDead() const;
	bool IsInvincible() const;
	bool IsHitWindowOpen() const;
	bool IsComboInputWindowOpen() const;
	bool HasBufferedComboInput() const;
	int GetCurrentComboIndex() const;
	int32 GetHitActorCountThisAttack() const;
	const FCombatAttackEntry* GetCurrentAttackEntry() const;
	ECombatActionState GetCombatActionState() const;
	
	void BeginHitWindow();
	void EndHitWindow();
	
	void BeginComboInputWindow();
	void EndComboInputWindow();
	
	bool RequestHitReaction(const FCombatDamageInfo& DamageInfo);
	bool IsHitReacting() const;
	
	bool IsDeathMontageFinished() const;
	
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
	bool StartAttack(ECombatAttackInputType AttackInputType);
	bool StartDodge(const FVector& DodgeDirection);
	
	bool TryPlayDodgeMontage();
	void ApplyDodgeMovement(const FVector& DodgeDirection);
	
	void BeginDodgeInvincibility();
	void EndDodgeInvincibility();
	
	bool TryBufferComboInput();
	bool TryCommitBufferedCombo();
	bool CanMoveToNextCombo() const;
	const FCombatAttackEntry* GetCurrentComboAttackData() const;
	const FCombatAttackEntry* GetComboAttackDataByIndex(int32 ComboIndex) const;
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
	
	void SetDodgeInvincible(bool bNewInvincible);
	void SetHitReactionInvincible(bool bNewInvincible);
	void BroadcastInvincibilityIfChanged(bool bOldInvincible);
	
	void PerformAttackTrace();
	void ApplyDamageToHitActor(const FHitResult& HitResult);
	bool HasAlreadyHitActor(const AActor* HitActor) const;
	void RegisterHitActor(AActor* HitActor);
	FVector GetAttackTraceStartLocation() const;
	FVector GetAttackTraceEndLocation() const;
	
	bool IsDamageBlockedByInvincibility(const AActor* TargetActor) const;

	void EndHitReaction();
	bool TryPlayHitReactionMontage(const FCombatDamageInfo& DamageInfo);
	UAnimMontage* GetHitReactionMontageByDirection(ECombatHitDirection HitDirection) const;
	void StartHitReactionInvincibility();
	void EndHitReactionInvincibility();
	void HandleHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	bool TryPlayDeathMontage();
	void HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void CancelCurrentActionForInterrupt();
	void CancelAttack();
	void ResetAttackHitState();
	
	UCombatAttackData* GetAttackDataByInputType(ECombatAttackInputType AttackInputType) const;
	UAnimMontage* GetAttackMontageByInputType(ECombatAttackInputType AttackInputType) const;
	
	UStaminaComponent* GetOwnerStaminaComponent() const;
	bool CanPayAttackStaminaCost(const FCombatAttackEntry& AttackEntry) const;
	bool TrySpendAttackStaminaCost(const FCombatAttackEntry& AttackEntry) const;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> LightAttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HeavyAttackMontage;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentAttackMontage;
	
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatAttackData> LightAttackData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatAttackData> HeavyAttackData;
	
	UPROPERTY()
	TObjectPtr<UCombatAttackData> CurrentCombatAttackData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawAttackTraceDebug = true;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	ECombatActionState CombatActionState = ECombatActionState::Idle;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	bool bHitWindowOpen = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	bool bDodgeInvincible = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	bool bHitReactionInvincible = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	int32 CurrentComboIndex = 0;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	bool bComboInputWindowOpen = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	bool bComboInputBuffered = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float HitReactionInvincibleDuration = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> FrontHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> BackHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> LeftHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> RightHitReactionMontage;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> CurrentHitReactionMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float HitReactionMontagePlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float DeathMontagePlayRate = 1.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Death", meta = (AllowPrivateAccess = "true"))
	bool bDeathMontageFinished = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	bool bCurrentAttackStaminaCostPaid = false;
	
private:
	FTimerHandle DodgeFallbackTimerHandle;
	FTimerHandle InvincibilityTimerHandle;
	
	FTimerHandle HitReactionInvincibleTimerHandle;
	
	TArray<TWeakObjectPtr<AActor>> HitActorsThisAttack;
};
