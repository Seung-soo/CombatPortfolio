// Fill out your copyright notice in the Description page of Project Settings.


#include "HitStopComponent.h"

UHitStopComponent::UHitStopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHitStopComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndHitStop();
	
	Super::EndPlay(EndPlayReason);
}

void UHitStopComponent::RequestHitStop(float Duration, float TimeDilation)
{
	if (0.0f >= Duration)
	{
		return;
	}
	
	AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	if (false == bHitStopActive)
	{
		OriginalCustomTimeDilation = OwnerActor->CustomTimeDilation;
		bHitStopActive = true;
	}
	
	OwnerActor->CustomTimeDilation = FMath::Max(TimeDilation, KINDA_SMALL_NUMBER);
	
	World->GetTimerManager().ClearTimer(HitStopTimerHandle);
	World->GetTimerManager().SetTimer(
		HitStopTimerHandle,
		this,
		&UHitStopComponent::EndHitStop,
		Duration,
		false
	);
}

void UHitStopComponent::EndHitStop()
{
	AActor* OwnerActor = GetOwner();
	
	if (nullptr != OwnerActor)
	{
		OwnerActor->CustomTimeDilation = OriginalCustomTimeDilation;
	}
	
	bHitStopActive = false;
}