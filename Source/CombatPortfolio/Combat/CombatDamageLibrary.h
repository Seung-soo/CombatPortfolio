// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatDamageType.h"
#include "CombatDamageLibrary.generated.h"

/**
 * 
 */
UCLASS()
class COMBATPORTFOLIO_API UCombatDamageLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Combat|Damage")
	static ECombatHitDirection CalculateHitDirectionFromActors(const AActor* HitActor, const AActor* InstigatorActor);
	
	UFUNCTION(BlueprintPure, Category = "Combat|Damage")
	static ECombatHitDirection CalculateHitDirectionFromIncomingDirection(const AActor* HitActor, const FVector& IncomingDirection);
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
	static bool ApplyKnockbackFromDamageInfo(const FCombatDamageInfo& DamageInfo);
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
	static void ApplyHitStopFromDamageInfo(const FCombatDamageInfo& DamageInfo);
};
