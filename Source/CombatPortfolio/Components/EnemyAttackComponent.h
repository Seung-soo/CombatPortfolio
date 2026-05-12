// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "CombatPortfolio/Combat/CombatDamageType.h"
#include "EnemyAttackComponent.generated.h"

class UAnimMontage;
class UCombatComponent;
class UHealthComponent;

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UEnemyAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyAttackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	void StartAutoAttack();
	void StopAutoAttack();
	void AttackOnce();
	
	bool RequestAttack();
	bool CanRequestAttack() const;
	bool IsAttackOnCooldown() const;
	bool IsAttacking() const;
	bool IsHitWindowOpen() const;
	void SetStartAttackOnBeginPlay(bool bNewStartAttackOnBeginPlay);
	
	void OpenAttackHitWindow();
	void CloseAttackHitWindow();
	void EndAttack();
	void CancelAttack();
	
private:
	bool TryPlayAttackMontage();
	void PerformAttackTrace();
	void ApplyDamageToActor(const FHitResult& HitResult);
	
	bool IsDamageBlockedByInvincibility(AActor* TargetActor) const;
	
	bool HasAlreadyHitActor(const AActor* TargetActor) const;
	
	void AddHitActor(AActor* TargetActor);
	void ResetHitActors();
	
	FVector GetAttackTraceStartLocation() const;
	FVector GetAttackTraceEndLocation() const;
	
	void BeginAttackCooldown();
	void EndAttackCooldown();
	
	void SetComponentTickByHitWindow();
	
	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	bool bStartAttackOnBeginPlay = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float AttackInterval = 2.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackDamage = 25.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackCooldown = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	ECombatHitStrength HitStrength = ECombatHitStrength::Light;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Animation", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float AttackMontagePlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Trace", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float AttackTraceRadius = 90.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Trace", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackTraceForwardOffset = 140.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Trace", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackTraceHalfHeight = 50.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawAttackDebug = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	bool bAttackOnCooldown = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	bool bAttacking = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	bool bHitWindowOpen = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float KnockbackStrength = 250.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float HitStopDuration = 0.04f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ClampMax = "1.0"))
	float HitStopTimeDilation = 0.05f;
	
private:
	TArray<TWeakObjectPtr<AActor>> HitActorsThisAttack;
	
	FTimerHandle AutoAttackTimerHandle;
	FTimerHandle AttackCooldownTimerHandle;
};
