// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatPortfolio/Characters/Enemy/CombatEnemyBase.h"
#include "CombatDummyEnemy.generated.h"

UCLASS()
class COMBATPORTFOLIO_API ACombatDummyEnemy : public ACombatEnemyBase
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACombatDummyEnemy();

protected:
	virtual void ApplyDeathState() override;
};
