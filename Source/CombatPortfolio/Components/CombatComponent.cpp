// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}

bool UCombatComponent::RequestAttack()
{
	if (false == CanStartAttack())
	{
		return false;
	}
	
	StartAttack();
	return true;
}

bool UCombatComponent::CanStartAttack() const
{
	return ECombatActionState::Idle == CombatActionState;
}

bool UCombatComponent::IsAttacking() const
{
	return ECombatActionState::Attacking == CombatActionState;
}

ECombatActionState UCombatComponent::GetCombatActionState() const
{
	return CombatActionState;
}

void UCombatComponent::StartAttack()
{
	CombatActionState = ECombatActionState::Attacking;
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		FinishAttack();
		return;
	}
	
	World->GetTimerManager().SetTimer(AttackTimerHandle, this, &UCombatComponent::FinishAttack, AttackDuration, false);
}

void UCombatComponent::FinishAttack()
{
	CombatActionState = ECombatActionState::Idle;
}