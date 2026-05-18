// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_PlayerParryWindow.h"

#include "CombatPortfolio/Components/CombatComponent.h"

void UAnimNotifyState_PlayerParryWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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
	
	UCombatComponent* CombatComponent = OwnerActor->FindComponentByClass<UCombatComponent>();
	
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	CombatComponent->BeginParryWindow();
}

void UAnimNotifyState_PlayerParryWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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
	
	UCombatComponent* CombatComponent = OwnerActor->FindComponentByClass<UCombatComponent>();
	
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	CombatComponent->EndParryWindow();
}
