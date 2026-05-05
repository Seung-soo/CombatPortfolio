// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class COMBATPORTFOLIO_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeHealth(float CurrentHealth, float MaxHealth);
	void SetHealth(float CurrentHealth, float MaxHealth);
	
private:
	float CalculateHealthRatio(float CurrentHealth, float MaxHealth) const;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthText;
};
