// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_EnemyAttackHitWindow.h"

#include "CombatPortfolio/Components/EnemyAttackComponent.h"

void UAnimNotifyState_EnemyAttackHitWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                        float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (nullptr == MeshComp)
	{
		return;
	}
	
	AActor* OwnerActor = MeshComp->GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	UEnemyAttackComponent* EnemyAttackComponent = OwnerActor->FindComponentByClass<UEnemyAttackComponent>();
	
	if (nullptr == EnemyAttackComponent)
	{
		return;
	}
	
	EnemyAttackComponent->OpenAttackHitWindow();
}

void UAnimNotifyState_EnemyAttackHitWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (nullptr == MeshComp)
	{
		return;
	}
	
	AActor* OwnerActor = MeshComp->GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	UEnemyAttackComponent* EnemyAttackComponent = OwnerActor->FindComponentByClass<UEnemyAttackComponent>();
	
	if (nullptr == EnemyAttackComponent)
	{
		return;
	}
	
	EnemyAttackComponent->CloseAttackHitWindow();
}
