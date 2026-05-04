// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CombatDummyEnemy.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class UHealthComponent;
class UEnemyAttackComponent;

UCLASS()
class COMBATPORTFOLIO_API ACombatDummyEnemy : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACombatDummyEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UEnemyAttackComponent> EnemyAttackComponent;
	
private:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta);
	
	UFUNCTION()
	void HandleDeath();
	
	void ApplyDeathState();
};
