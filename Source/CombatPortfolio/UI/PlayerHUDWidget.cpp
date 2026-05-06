// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPlayerHUDWidget::InitializeHUD(float CurrentHealth, float MaxHealth, float CurrentStamina, float MaxStamina)
{
	SetHealth(CurrentHealth, MaxHealth);
	SetStamina(CurrentStamina, MaxStamina);
	HideDeathMessage();
}

void UPlayerHUDWidget::SetHealth(float CurrentHealth, float MaxHealth)
{
	const float HealthRatio = CalculateRatio(CurrentHealth, MaxHealth);
	
	if (nullptr != HealthProgressBar)
	{
		HealthProgressBar->SetPercent(HealthRatio);
	}
	
	if (nullptr != HealthText)
	{
		const FString HealthString = FString::Printf(TEXT("HP %.0f / %.0f"), CurrentHealth, MaxHealth);
		
		HealthText->SetText(FText::FromString(HealthString));
	}
}

void UPlayerHUDWidget::SetStamina(float CurrentStamina, float MaxStamina)
{
	const float StaminaRatio = CalculateRatio(CurrentStamina, MaxStamina);
	
	if (nullptr != StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(StaminaRatio);
	}
	
	if (nullptr != StaminaText)
	{
		const FString StaminaString = FString::Printf(TEXT("ST %.0f / %.0f"), CurrentStamina, MaxStamina);
		
		StaminaText->SetText(FText::FromString(StaminaString));
	}
}

void UPlayerHUDWidget::ShowDeathMessage()
{
	if (nullptr != DeathText)
	{
		DeathText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UPlayerHUDWidget::HideDeathMessage()
{
	if (nullptr != DeathText)
	{
		DeathText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

float UPlayerHUDWidget::CalculateRatio(float CurrentValue, float MaxValue) const
{
	if (0.0f >= MaxValue)
	{
		return 0.0f;
	}
	
	return FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f);
}
