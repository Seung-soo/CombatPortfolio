// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatDamageType.generated.h"

UENUM(BlueprintType)
enum class ECombatHitStrength : uint8
{
	Light UMETA(DisplayName = "Light"),
	Medium UMETA(DisplayName = "Medium"),
	Heavy UMETA(DisplayName = "Heavy"),
};

static const TCHAR* GetCombatHitStrengthDebugString(ECombatHitStrength HitStrength)
{
	switch (HitStrength)
	{
	case ECombatHitStrength::Light: return TEXT("Light");
	case ECombatHitStrength::Medium: return TEXT("Medium");
	case ECombatHitStrength::Heavy: return TEXT("Heavy");
	default: return TEXT("Unknown");
	}
}

UENUM(BlueprintType)
enum class ECombatHitDirection : uint8
{
	Front UMETA(DisplayName = "Front"),
	Back UMETA(DisplayName = "Back"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

static const TCHAR* GetCombatHitDirectionDebugString(ECombatHitDirection HitDirection)
{
	switch (HitDirection)
	{
	case ECombatHitDirection::Front: return TEXT("Front");
	case ECombatHitDirection::Back: return TEXT("Back");
	case ECombatHitDirection::Left: return TEXT("Left");
	case ECombatHitDirection::Right: return TEXT("Right");
	default: return TEXT("Unknown");
	}
}

USTRUCT(BlueprintType)
struct FCombatDamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
	float DamageAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	TObjectPtr<AActor> DamageCauser = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	FVector HitNormal = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	FVector HitDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	ECombatHitStrength HitStrength = ECombatHitStrength::Light;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	ECombatHitDirection HitDirectionType = ECombatHitDirection::Front;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
	float KnockbackStrength = 0.0f;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
	float HitStopDuration = 0.04f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float HitStopTimeDilation = 0.05f;
};
