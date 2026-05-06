// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class COMBATPORTFOLIO_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeHUD(float CurrentHealth, float MaxHealth, float CurrentStamina, float MaxStamina);
	
	void SetHealth(float CurrentHealth, float MaxHealth);
	void SetStamina(float CurrentStamina, float MaxStamina);
	
	void ShowDeathMessage();
	void HideDeathMessage();
	
private:
	float CalculateRatio(float CurrentValue, float MaxValue) const;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaProgressBar;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StaminaText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DeathText;
};
