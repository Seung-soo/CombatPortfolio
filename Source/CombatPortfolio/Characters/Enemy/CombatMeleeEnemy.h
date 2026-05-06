// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatEnemyBase.h"
#include "CombatMeleeEnemy.generated.h"

UENUM(BlueprintType)
enum class EMeleeEnemyState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Chasing UMETA(DisplayName = "Chasing"),
	Attacking UMETA(DisplayName = "Attacking"),
	Dead UMETA(DisplayName = "Dead")
};

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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawMeleeDebug = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee AI", meta = (AllowPrivateAccess = "true"))
	EMeleeEnemyState MeleeEnemyState = EMeleeEnemyState::Idle;
	
	
private:
	TWeakObjectPtr<APawn> TargetPlayerPawn;
	
};
