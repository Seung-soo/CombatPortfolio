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
	if (DamageAmount <= 0.0f)
	{
		return false;
	}
	
	if (true == IsDead())
	{
		return false;
	}
	
	const float NewHealth = CurrentHealth - DamageAmount;
	SetCurrentHealth(NewHealth);
	
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
	
	if (0.0f >= CurrentHealth && false == bDead)
	{
		bDead = true;
		
		const AActor* OwnerActor = GetOwner();
		const FString OwnerName = nullptr != OwnerActor ? OwnerActor->GetName() : TEXT("UnknownOwner");
		
		UE_LOG(LogTemp, Log, TEXT("Owner died: %s"), *OwnerName);
		
		OnDeath.Broadcast();
	}
}
