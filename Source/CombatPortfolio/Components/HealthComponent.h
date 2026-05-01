// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedSignature, 
	float, CurrentHealth, float, MaxHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	bool ApplyDamage(float DamageAmount);
	
	float GetCurrentHealth();
	float GetMaxHealth();
	float GetHealthRatio();
	
	bool IsDead();
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Health|Event")
	FOnHealthChangedSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Health|Event")
	FOnDeathSignature OnDeath;
	
private:
	void SetCurrentHealth(float NewHealth);
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	float MaxHealth = 100.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	float CurrentHealth = 0.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	bool bDead = false;
};
