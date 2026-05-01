// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatAnimNotifyState_ComboInputWindow.h"
#include "CombatPortfolio/Components/CombatComponent.h"

void UCombatAnimNotifyState_ComboInputWindow::NotifyBegin(USkeletalMeshComponent* MeshComp,
                                                          UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
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
	
	CombatComponent->BeginComboInputWindow();
}

void UCombatAnimNotifyState_ComboInputWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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
	
	CombatComponent->EndComboInputWindow();
}
