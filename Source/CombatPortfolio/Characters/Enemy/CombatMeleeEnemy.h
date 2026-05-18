// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatEnemyBase.h"
#include "CombatPortfolio/Combat/CombatDamageType.h"
#include "CombatMeleeEnemy.generated.h"

UENUM(BlueprintType)
enum class EMeleeEnemyState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Chasing UMETA(DisplayName = "Chasing"),
	Attacking UMETA(DisplayName = "Attacking"),
	HitReacting UMETA(DisplayName = "HitReacting"),
	Parried UMETA(DisplayName = "Parried"),
	Dead UMETA(DisplayName = "Dead")
};

class UAnimMontage;

/**
 * 
 */
UCLASS()
class COMBATPORTFOLIO_API ACombatMeleeEnemy : public ACombatEnemyBase
{
	GENERATED_BODY()
	
public:
	ACombatMeleeEnemy();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void ApplyDeathState() override;
	virtual void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta) override;
	
public:
	bool RequestParriedReaction();
	
private:
	void CachePlayerPawn();
	bool HasValidTarget() const;
	bool IsTargetDead() const;
	float GetDistanceToTarget() const;
	bool IsTargetInsideDetectionRadius() const;
	bool IsTargetInsideAttackRange() const;
	bool IsTargetInsideStopDistance() const;
	bool IsFacingTarget() const;
	
	FVector GetPlanarDirectionToTarget() const;
	void UpdateFacingToTarget(float DeltaTime);
	void UpdateChaseMovement();
	void StopChaseMovement();
	void TryAttackTarget();
	
	UFUNCTION()
	void HandleDamaged(const FCombatDamageInfo& DamageInfo);
	
	void StartHitReaction(const FCombatDamageInfo& DamageInfo);
	void EndHitReaction();
	bool IsHitReacting() const;
	bool TryPlayHitReactionMontage(const FCombatDamageInfo& DamageInfo);
	UAnimMontage* GetHitReactionMontageByDirection(ECombatHitDirection HitDirection) const;
	
	void HandleHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	
	void StartParriedReaction();
	void EndParriedReaction();
	bool IsParriedReacting() const;
	bool TryPlayParriedMontage();
	void HandleParriedMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void SetMeleeEnemyState(EMeleeEnemyState NewState);
	
	FString GetMeleeEnemyStateDebugString() const;
	
	void DrawMeleeDebug() const;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DetectionRadius = 900.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackRange = 240.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float StopDistance = 180.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Navigation", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float NavigationAcceptanceRadius = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ChaseMoveSpeed = 280.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FacingRotationInterpSpeed = 8.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "180.0"))
	float FacingAngleTolerance = 20.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> FrontHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> BackHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> LeftHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> RightHitReactionMontage;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Melee AI|Hit Reaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> CurrentHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Hit Reaction", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float HitReactionMontagePlayRate = 1.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Melee AI|Hit Reaction", meta = (AllowPrivateAccess = "true"))
	bool bHitReacting = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawMeleeDebug = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI", meta = (AllowPrivateAccess = "true"))
	EMeleeEnemyState MeleeEnemyState = EMeleeEnemyState::Idle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Parry", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ParriedMontage;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Melee AI|Parry", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> CurrentParriedMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Parry", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float ParriedMontagePlayRate = 1.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Melee AI|Parry", meta = (AllowPrivateAccess = "true"))
	bool bParriedReacting = false;
	
private:
	TWeakObjectPtr<APawn> TargetPlayerPawn;
};
