// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHUDComponent.generated.h"

class UPlayerHUDWidget;
class UPlayerHUDComponent;
class UHealthComponent;
class UStaminaComponent;

UCLASS( ClassGroup=(UI), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UPlayerHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerHUDComponent();

protected:
	virtual void BeginPlay() override;

public:
	void InitializeHUD();
	void ShowDeathMessage();
	
private:
	void CacheOwnerComponents();
	void CreatePlayerHUD();
	
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta);
	
	UFUNCTION()
	void HandleStaminaChanged(float CurrentStamina, float MaxStamina, float Delta);
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerHUDWidget> PlayerHUDWidget;
	
	UPROPERTY()
	TObjectPtr<UHealthComponent> HealthComponent;
	
	UPROPERTY()
	TObjectPtr<UStaminaComponent> StaminaComponent;
		
};
