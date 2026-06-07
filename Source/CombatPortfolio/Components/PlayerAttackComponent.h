// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatPortfolio/Data/CombatAttackData.h"
#include "PlayerAttackComponent.generated.h"

class UAnimMontage;
class UAnimInstance;
class UStaminaComponent;
class ACombatEnemyBase;

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UPlayerAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerAttackComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	bool StartAttack(ECombatAttackInputType AttackInputType);
	bool StartCriticalAttack(AActor* CriticalTargetActor);
	bool TryBufferComboInput();
	void CancelAttack();

	void BeginHitWindow();
	void EndHitWindow();
	
	void BeginComboInputWindow();
	void EndComboInputWindow();
	
	bool IsAttacking() const;
	bool IsHitWindowOpen() const;
	bool IsComboInputWindowOpen() const;
	bool HasBufferedComboInput() const;
	
	int GetCurrentComboIndex() const;
	int32 GetHitActorCountThisAttack() const;
	const FCombatAttackEntry* GetCurrentAttackEntry() const;

public:
	FSimpleMulticastDelegate OnAttackFinished;
	FSimpleMulticastDelegate OnHitWindowChanged;
	FSimpleMulticastDelegate OnComboStateChanged;

private:
	void FinishAttack();
	
	bool TryCommitBufferedCombo();
	bool CanMoveToNextCombo() const;
	
	const FCombatAttackEntry* GetCurrentComboAttackData() const;
	const FCombatAttackEntry* GetComboAttackDataByIndex(int32 ComboIndex) const;
	
	FName GetCurrentComboSectionName() const;
	void ResetComboState();
	
	UAnimInstance* GetOwnerAnimInstance() const;
	
	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void PerformAttackTrace();
	void ApplyDamageToHitActor(const FHitResult& HitResult);
	
	bool HasAlreadyHitActor(const AActor* HitActor) const;
	void RegisterHitActor(AActor* HitActor);
	
	FVector GetAttackTraceStartLocation() const;
	FVector GetAttackTraceEndLocation() const;
	
	UCombatAttackData* GetAttackDataByInputType(ECombatAttackInputType AttackInputType) const;
	UAnimMontage* GetAttackMontageByInputType(ECombatAttackInputType AttackInputType) const;
	
	UStaminaComponent* GetOwnerStaminaComponent() const;
	bool CanPayAttackStaminaCost(const FCombatAttackEntry& AttackEntry) const;
	bool TrySpendAttackStaminaCost(const FCombatAttackEntry& AttackEntry) const;
	
	void SetHitWindowOpen(bool bNewHitWindowOpen);
	void SetComboInputWindowOpen(bool bNewComboInputWindowOpen);
	void SetComboInputBuffered(bool bNewComboInputBuffered);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> LightAttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HeavyAttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack|Critical", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> CriticalAttackMontage;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentAttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	float AttackPlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatAttackData> LightAttackData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatAttackData> HeavyAttackData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatAttackData> CriticalAttackData;
	
	UPROPERTY()
	TObjectPtr<UCombatAttackData> CurrentCombatAttackData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawAttackTraceDebug = true;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	bool bAttackActive = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	bool bHitWindowOpen = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	int32 CurrentComboIndex = 0;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	bool bComboInputWindowOpen = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	bool bComboInputBuffered = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	bool bCurrentAttackStaminaCostPaid = false;
	
private:
	TArray<TWeakObjectPtr<AActor>> HitActorsThisAttack;
	
	UPROPERTY()
	TWeakObjectPtr<ACombatEnemyBase> CurrentCriticalTarget;
};
