// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "EnemyHealthBarComponent.generated.h"

class UEnemyHealthBarWidget;

/**
 * 
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class COMBATPORTFOLIO_API UEnemyHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UEnemyHealthBarComponent();
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	void InitializeHealth(float CurrentHealth, float MaxHealth);
	void SetHealth(float CurrentHealth, float MaxHealth);
	void ShowHealthBar();
	void HideHealthBar();
	
private:
	void UpdateHealthBarWorldLocation();
	UEnemyHealthBarWidget* GetEnemyHealthBarWidget() const;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Health Bar", meta = (AllowPrivateAccess = "true"))
	float HealthBarHeight = 105.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Health Bar", meta = (AllowPrivateAccess = "true"))
	bool bHideWhenFullHealth = false;
};
