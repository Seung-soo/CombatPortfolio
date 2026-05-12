// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatEnemyBase.generated.h"

class UStaticMeshComponent;
class UHealthComponent;
class UEnemyAttackComponent;
class ULockOnMarkerComponent;
class UEnemyHealthBarComponent;
class UHitStopComponent;

UCLASS()
class COMBATPORTFOLIO_API ACombatEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACombatEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
protected:
	UFUNCTION()
	virtual void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta);
	
	UFUNCTION()
	virtual void HandleDeath();
	
	virtual void ApplyDeathState();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> BodyMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UEnemyAttackComponent> EnemyAttackComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<ULockOnMarkerComponent> LockOnMarkerComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UEnemyHealthBarComponent> EnemyHealthBarComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UHitStopComponent> HitStopComponent;

public:
	UHealthComponent* GetHealthComponent() const;
	UEnemyAttackComponent* GetEnemyAttackComponent() const;
	ULockOnMarkerComponent* GetLockOnMarkerComponent() const;
	UEnemyHealthBarComponent* GetEnemyHealthBarComponent() const;
};
