// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bDead = false;
}

bool UHealthComponent::ApplyDamage(float DamageAmount)
{
	FCombatDamageInfo DamageInfo;
	DamageInfo.DamageAmount = DamageAmount;
	DamageInfo.HitActor = GetOwner();
	
	return ApplyDamage(DamageInfo);
}

bool UHealthComponent::ApplyDamage(const FCombatDamageInfo& DamageInfo)
{
	if (0.0f >= DamageInfo.DamageAmount)
	{
		return false;
	}
	
	if (true == IsDead())
	{
		return false;
	}
	
	if (nullptr != DamageInfo.HitActor && DamageInfo.HitActor != GetOwner())
	{
		return false;
	}
	
	const float OldHealth = CurrentHealth;
	const float NewHealth = CurrentHealth - DamageInfo.DamageAmount;
	
	SetCurrentHealth(NewHealth);
	
	if (true == FMath::IsNearlyEqual(OldHealth, CurrentHealth))
	{
		return false;
	}
	
	OnDamaged.Broadcast(DamageInfo);
	
	UE_LOG(LogTemp, Log, TEXT("DamageInfo | Damage: %.1f | Knockback: %.1f | Instigator: %s | Target: %s | Strength: %s | Direction: %s"),
		DamageInfo.DamageAmount,
		DamageInfo.KnockbackStrength,
		*GetNameSafe(DamageInfo.InstigatorActor),
		*GetNameSafe(DamageInfo.HitActor),
		GetCombatHitStrengthDebugString(DamageInfo.HitStrength),
		GetCombatHitDirectionDebugString(DamageInfo.HitDirectionType));
	
	TryHandleDeath();
	
	return true;
}

float UHealthComponent::GetCurrentHealth()
{
	return CurrentHealth;
}

float UHealthComponent::GetMaxHealth()
{
	return MaxHealth;
}

float UHealthComponent::GetHealthRatio()
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	
	return CurrentHealth / MaxHealth;
}

bool UHealthComponent::IsDead()
{
	return bDead;
}

void UHealthComponent::SetCurrentHealth(float NewHealth)
{
	const float OldHealth = CurrentHealth;
	
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	
	if (true == FMath::IsNearlyEqual(OldHealth, CurrentHealth))
	{
		return;
	}
	
	const float Delta = CurrentHealth - OldHealth;
	
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, Delta);
	
	UE_LOG(LogTemp, Log, TEXT("Health changed: %.1f / %.1f, Delta: %.1f"), CurrentHealth, MaxHealth, Delta);
}

void UHealthComponent::TryHandleDeath()
{
	if (0.0f < CurrentHealth)
	{
		return;
	}
	
	if (true == bDead)
	{
		return;
	}
	
	bDead = true;

	const AActor* OwnerActor = GetOwner();
	const FString OwnerName = nullptr != OwnerActor ? OwnerActor->GetName() : TEXT("UnknownOwner");

	UE_LOG(LogTemp, Log, TEXT("Owner died: %s"), *OwnerName);

	OnDeath.Broadcast();
\
}
