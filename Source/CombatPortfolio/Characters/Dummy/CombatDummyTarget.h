// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CombatDummyTarget.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class UHealthComponent;

UCLASS()
class COMBATPORTFOLIO_API ACombatDummyTarget : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACombatDummyTarget();

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
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = true))
	float HitReactionScaleMultiplier = 1.15f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = true))
	float HitReactionDuration = 0.12f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (AllowPrivateAccess = true))
	bool bDisableCollisionOnDeath = true;
	
private:
	FVector OriginalMeshScale = FVector::OneVector; 
	
	FTimerHandle HitReactionTimerHandle;
	
private:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta);
	
	UFUNCTION()
	void HandleDeath();
	
	void PlayHitReaction();
	void ResetHitReaction();
	void ApplyDeathState();
};
