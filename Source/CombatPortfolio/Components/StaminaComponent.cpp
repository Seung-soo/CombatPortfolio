// Fill out your copyright notice in the Description page of Project Settings.


#include "StaminaComponent.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


// Called when the game starts
void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentStamina = MaxStamina;
	bDrainingStamina = false;
	bRegenEnabled = false;
	ActiveDrainRate = 0.0f;
	
	SetComponentTickEnabled(false);
}

// Called every frame
void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (true == bDrainingStamina)
	{
		ConsumeStamina(ActiveDrainRate * DeltaTime);
		
		if (true == IsStaminaEmpty())
		{
			bDrainingStamina = false;
			ActiveDrainRate = 0.0f;
			ScheduleStaminaRegen();
		}
		
		UpdateComponentTickEnabled();
		return;
	}
	
	if (true == bRegenEnabled && CurrentStamina < MaxStamina)
	{
		RestoreStamina(StaminaRegenRate * DeltaTime);
		
		if (CurrentStamina >= MaxStamina)
		{
			bRegenEnabled = false;
		}
		
		UpdateComponentTickEnabled();
	}
}

bool UStaminaComponent::TrySpendStamina(float StaminaAmount)
{
	if (0.0f >= StaminaAmount)
	{
		return true;
	}
	
	if (false == HasEnoughStamina(StaminaAmount))
	{
		return false;
	}
	
	ConsumeStamina(StaminaAmount);
	ScheduleStaminaRegen();
	
	return true;
}

bool UStaminaComponent::HasEnoughStamina(float StaminaAmount) const
{
	return CurrentStamina >= StaminaAmount;
}

bool UStaminaComponent::StartStaminaDrain(float DrainRate)
{
	if (0.0f >= DrainRate)
	{
		return false;
	}
	
	if (true == IsStaminaEmpty())
	{
		return false;
	}
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		World->GetTimerManager().ClearTimer(StaminaRegenDelayTimerHandle);
	}
	
	bDrainingStamina = true;
	bRegenEnabled = false;
	ActiveDrainRate = DrainRate;
	
	UpdateComponentTickEnabled();
	
	return true;
}

void UStaminaComponent::StopStaminaDrain()
{
	if (false == bDrainingStamina)
	{
		return;
	}
	
	bDrainingStamina = false;
	ActiveDrainRate = 0.0f;
	
	ScheduleStaminaRegen();
}

float UStaminaComponent::GetCurrentStamina() const
{
	return CurrentStamina;
}

float UStaminaComponent::GetMaxStamina() const
{
	return MaxStamina;
}

float UStaminaComponent::GetStaminaRatio() const
{
	if (0.0f >= MaxStamina)
	{
		return 0.0f;
	}
	
	return (CurrentStamina / MaxStamina);
}

bool UStaminaComponent::IsStaminaEmpty() const
{
	return 0.0f >= CurrentStamina;
}

bool UStaminaComponent::IsDrainingStamina() const
{
	return bDrainingStamina;
}

void UStaminaComponent::ConsumeStamina(float StaminaAmount)
{
	if (0.0f >= StaminaAmount)
	{
		return;
	}
	
	SetCurrentStamina(CurrentStamina - StaminaAmount);
}

void UStaminaComponent::RestoreStamina(float StaminaAmount)
{
	if (0.0f >= StaminaAmount)
	{
		return;
	}
	
	SetCurrentStamina(CurrentStamina + StaminaAmount);
}

void UStaminaComponent::SetCurrentStamina(float NewStamina)
{
	const float OldStamina = CurrentStamina;
	
	CurrentStamina = FMath::Clamp(NewStamina, 0.0f, MaxStamina);
	
	if (true == FMath::IsNearlyEqual(OldStamina, CurrentStamina))
	{
		return;
	}
	
	const float Delta = CurrentStamina - OldStamina;
	
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, Delta);
	
	UE_LOG(LogTemp, Log, TEXT("Stamina changed: %.1f / %.1f, Delta: %.1f"), CurrentStamina, MaxStamina, Delta);
	
	if (0.0f >= CurrentStamina && 0.0f < OldStamina)
	{
		OnStaminaDepleted.Broadcast();
		
		UE_LOG(LogTemp, Log, TEXT("Stamina depleted"));
	}
}

void UStaminaComponent::ScheduleStaminaRegen()
{
	bRegenEnabled = false;
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(StaminaRegenDelayTimerHandle);
	
	World->GetTimerManager().SetTimer(
		StaminaRegenDelayTimerHandle,
		this,
		&UStaminaComponent::BeginStaminaRegen,
		StaminaRegenDelay,
		false
	);
	
	UpdateComponentTickEnabled();
}

void UStaminaComponent::BeginStaminaRegen()
{
	if (CurrentStamina >= MaxStamina)
	{
		bRegenEnabled = false;
		UpdateComponentTickEnabled();
		return;
	}
	
	bRegenEnabled = true;
	UpdateComponentTickEnabled();
}

void UStaminaComponent::UpdateComponentTickEnabled()
{
	const bool bShouldTick = true == bDrainingStamina || (true == bRegenEnabled && CurrentStamina < MaxStamina);
	
	SetComponentTickEnabled(bShouldTick);
}

