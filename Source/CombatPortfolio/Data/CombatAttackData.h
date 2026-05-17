// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatPortfolio/Combat/CombatDamageType.h"
#include "Engine/DataAsset.h"
#include "CombatAttackData.generated.h"

UENUM(BlueprintType)
enum class ECombatAttackInputType : uint8
{
	Light UMETA(DisplayName = "Light"),
	Heavy UMETA(DisplayName = "Heavy")
};

USTRUCT(BlueprintType)
struct FCombatAttackEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FName SectionName = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	ECombatHitStrength HitStrength = ECombatHitStrength::Light;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float Damage = 25.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace", meta = (ClampMin = "1.0"))
	float TraceRadius = 80.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace", meta = (ClampMin = "0.0"))
	float TraceForwardOffset = 140.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace", meta = (ClampMin = "0.0"))
	float TraceHalfHeight = 40.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Knockback", meta = (ClampMin = "0.0"))
	float KnockbackStrength = 250.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|HitStop", meta = (ClampMin = "0.0"))
	float HitStopDuration = 0.04f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|HitStop", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float HitStopTimeDilation = 0.05f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Camera")
	TSubclassOf<UCameraShakeBase> HitCameraShakeClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Camera", meta = (ClampMin = "0.0"))
	float HitCameraShakeScale = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Feedback")
	TObjectPtr<UNiagaraSystem> HitVFX = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Feedback")
	TObjectPtr<USoundBase> HitSFX = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Feedback", meta = (ClampMin = "0.0"))
	float HitVFXScale = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Feedback", meta = (ClampMin = "0.0"))
	float HitSFXVolumeMultiplier = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Feedback", meta = (ClampMin = "0.0"))
	float HitSFXPitchMultiplier = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Stamina", meta = (ClampMin = "0.0"))
	float StaminaCost = 0.0f;
};

/**
 * 
 */
UCLASS(BlueprintType)
class COMBATPORTFOLIO_API UCombatAttackData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TArray<FCombatAttackEntry> Attacks;
};
