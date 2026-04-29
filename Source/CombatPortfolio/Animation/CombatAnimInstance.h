// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CombatAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class COMBATPORTFOLIO_API UCombatAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TObjectPtr<APawn> OwnerPawn;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling = false;
};
