// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "StaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFStaminaChangedSignature,
	float, CurrentStamina, float, MaxStamina, float, Delta	
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepletedSignature);


UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStaminaComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	bool TrySpendStamina(float StaminaAmount);
	bool HasEnoughStamina(float StaminaAmount) const;
	bool StartStaminaDrain(float DrainRate);
	void StopStaminaDrain();
	
	float GetCurrentStamina() const;
	float GetMaxStamina() const;
	float GetStaminaRatio() const;
	
	bool IsStaminaEmpty() const;
	bool IsDrainingStamina() const;
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Stamina|Event")
	FOnFStaminaChangedSignature OnStaminaChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Stamina|Event")
	FOnStaminaDepletedSignature OnStaminaDepleted;
	
private:
	void ConsumeStamina(float StaminaAmount);
	void RestoreStamina(float StaminaAmount);
	void SetCurrentStamina(float NewStamina);
	void ScheduleStaminaRegen();
	void BeginStaminaRegen();
	void UpdateComponentTickEnabled();
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = true, ClampMin = "1.0"))
	float MaxStamina = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
	float StaminaRegenRate = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
	float StaminaRegenDelay = 1.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = true))
	float CurrentStamina = 0.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = true))
	bool bDrainingStamina = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = true))
	bool bRegenEnabled = false;
	
private:
	float ActiveDrainRate = 0.0f;
	
	FTimerHandle StaminaRegenDelayTimerHandle;
};
