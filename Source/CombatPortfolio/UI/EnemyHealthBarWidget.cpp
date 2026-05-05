// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthBarWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UEnemyHealthBarWidget::InitializeHealth(float CurrentHealth, float MaxHealth)
{
	SetHealth(CurrentHealth, MaxHealth);
}

void UEnemyHealthBarWidget::SetHealth(float CurrentHealth, float MaxHealth)
{
	const float HealthRatio = CalculateHealthRatio(CurrentHealth, MaxHealth);
	
	if (nullptr != HealthProgressBar)
	{
		HealthProgressBar->SetPercent(HealthRatio);
	}
	
	if (nullptr != HealthText)
	{
		const FString HealthString = FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth);
		
		HealthText->SetText(FText::FromString(HealthString));
	}
}

float UEnemyHealthBarWidget::CalculateHealthRatio(float CurrentHealth, float MaxHealth) const
{
	if (0.0f >= MaxHealth)
	{
		return 0.0f;
	}
	
	return FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
}
