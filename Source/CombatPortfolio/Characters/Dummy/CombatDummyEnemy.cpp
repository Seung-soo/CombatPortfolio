// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatDummyEnemy.h"

#include "CombatPortfolio/Components/EnemyAttackComponent.h"
#include "CombatPortfolio/Components/EnemyHealthBarComponent.h"
#include "CombatPortfolio/Components/HealthComponent.h"
#include "CombatPortfolio/Components/LockOnMarkerComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ACombatDummyEnemy::ACombatDummyEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACombatDummyEnemy::ApplyDeathState()
{
	Super::ApplyDeathState();
	
	if (nullptr != BodyMeshComponent)
	{
		BodyMeshComponent->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.35f));
		BodyMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -85.0f));
	}
}
