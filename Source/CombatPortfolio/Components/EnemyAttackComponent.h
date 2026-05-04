// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "EnemyAttackComponent.generated.h"

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

public:	
	void StartAutoAttack();
	void StopAutoAttack();
	void AttackOnce();
	
private:
	void PerformAttackTrace();
	void ApplyDamageToActor(AActor* TargetActor);
	
	bool IsDamageBlockedByInvincibility(AActor* TargetActor) const;
	
	FVector GetAttackTraceStartLocation() const;
	FVector GetAttackTraceEndLocation() const;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	bool bStartAttackOnBeginPlay = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float AttackInterval = 2.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackDamage = 25.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Trace", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float AttackTraceRadius = 90.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Trace", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackTraceForwardOffset = 140.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Trace", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackTraceHalfHeight = 50.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawAttackDebug = true;
	
private:
	FTimerHandle AutoAttackTimerHandle;
};
