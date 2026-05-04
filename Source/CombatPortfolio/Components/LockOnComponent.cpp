// Fill out your copyright notice in the Description page of Project Settings.


#include "LockOnComponent.h"
#include "HealthComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "LockOnMarkerComponent.h"
#include "Engine/OverlapResult.h"

// Sets default values for this component's properties
ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


// Called when the game starts
void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);
}


// Called every frame
void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateLockOnValidation();
}

bool ULockOnComponent::ToggleLockOn()
{
	if (true == IsLockedOn())
	{
		ClearLockOnTarget();
		return false;
	}
	
	return TryLockOn();
}

bool ULockOnComponent::TryLockOn()
{
	AActor* BestTarget = FindBestLockOnTarget();
	
	if (nullptr == BestTarget)
	{
		UE_LOG(LogTemp, Log, TEXT("LockOn failed: no valid target."));
		return false;
	}
	
	SetLockOnTarget(BestTarget);
	
	UE_LOG(LogTemp, Log, TEXT("LockOn target: %s"), *BestTarget->GetName());
	
	return true;
}

void ULockOnComponent::ClearLockOnTarget()
{
	if (false == IsLockedOn())
	{
		return;
	}
	
	SetLockOnTarget(nullptr);
	
	UE_LOG(LogTemp, Log, TEXT("LockOn cleared."));
}

bool ULockOnComponent::IsLockedOn() const
{
	return LockOnTarget.IsValid();
}

AActor* ULockOnComponent::GetLockOnTarget() const
{
	return LockOnTarget.Get();
}

AActor* ULockOnComponent::FindBestLockOnTarget() const
{
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return nullptr;
	}
	
	AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return nullptr;
	}
	
	TArray<FOverlapResult> OverlapResults;
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	
	const bool bHasOverlap = World->OverlapMultiByChannel(
		OverlapResults,
		OwnerActor->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(LockOnRadius),
		QueryParams
	);
	
	if (true == bDrawLockOnDebug)
	{
		DrawDebugSphere(
			World,
			OwnerActor->GetActorLocation(),
			LockOnRadius,
			32,
			bHasOverlap ? FColor::Yellow : FColor::White,
			false,
			0.5f
		);
	}
	
	if (false == bHasOverlap)
	{
		return nullptr;
	}
	
	AActor* BestTarget = nullptr;
	float BestDistance = TNumericLimits<float>::Max();
	
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* CandidateActor = OverlapResult.GetActor();
		
		if (false == IsValidLockOnTarget(CandidateActor))
		{
			continue;
		}
		
		if (false == IsTargetInsideLockOnAngle(CandidateActor))
		{
			continue;
		}
		
		const float Distance = GetDistanceToTarget(CandidateActor);
		
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestTarget = CandidateActor;
		}
	}
	
	return BestTarget;
}

bool ULockOnComponent::IsValidLockOnTarget(const AActor* CandidateActor) const
{
	if (nullptr == CandidateActor)
	{
		return false;
	}
	
	const AActor* OwnerActor = GetOwner();
	
	if (CandidateActor == OwnerActor)
	{
		return false;
	}
	
	UHealthComponent* CandidateHealthComponent = CandidateActor->FindComponentByClass<UHealthComponent>();
	
	if (nullptr == CandidateHealthComponent)
	{
		return false;
	}
	
	if (true == CandidateHealthComponent->IsDead())
	{
		return false;
	}
	
	return true;
}

bool ULockOnComponent::IsTargetInsideLockOnAngle(const AActor* CandidateActor) const
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor || nullptr == CandidateActor)
	{
		return false;
	}
	
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector TargetLocation = CandidateActor->GetActorLocation();
	
	const FVector ToTargetDirection = (TargetLocation - OwnerLocation).GetSafeNormal2D();
	
	if (true == ToTargetDirection.IsNearlyZero())
	{
		return false;
	}
	
	const FVector OwnerForward = OwnerActor->GetActorForwardVector().GetSafeNormal2D();
	
	const float Dot = FVector::DotProduct(OwnerForward, ToTargetDirection);
	
	const float HalfAngleRadians = FMath::DegreesToRadians(LockOnAngle * 0.5f);
	const float MinDot = FMath::Cos(HalfAngleRadians);
	
	return Dot >= MinDot;
}

float ULockOnComponent::GetDistanceToTarget(const AActor* CandidateActor) const
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor || nullptr == CandidateActor)
	{
		return TNumericLimits<float>::Max();
	}
	
	return FVector::Dist(OwnerActor->GetActorLocation(), CandidateActor->GetActorLocation());
}

void ULockOnComponent::SetLockOnTarget(AActor* NewTarget)
{
	AActor* OldTarget = LockOnTarget.Get();
	
	if (OldTarget == NewTarget)
	{
		return;
	}
	
	HideTargetMarker(OldTarget);
	
	LockOnTarget = NewTarget;
	
	ShowTargetMarker(NewTarget);
	
	SetComponentTickEnabled(IsLockedOn());
	
	OnLockOnTargetChaged.Broadcast();
}

void ULockOnComponent::UpdateLockOnValidation()
{
	AActor* CurrentTarget = GetLockOnTarget();
	
	if (nullptr == CurrentTarget)
	{
		ClearLockOnTarget();
		return;
	}
	
	if (false == IsValidLockOnTarget(CurrentTarget))
	{
		ClearLockOnTarget();
		return;
	}
	
	const float Distance = GetDistanceToTarget(CurrentTarget);
	
	if (Distance > LockOnRadius)
	{
		ClearLockOnTarget();
		return;
	}
}

void ULockOnComponent::ShowTargetMarker(AActor* TargetActor) const
{
	if (nullptr == TargetActor)
	{
		return;
	}
	
	ULockOnMarkerComponent* MarkerComponent = TargetActor->FindComponentByClass<ULockOnMarkerComponent>();
	
	if (nullptr == MarkerComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("LockOn target has no LockOnMarkerComponent: %s"), *TargetActor->GetName());
		return;
	}
	
	MarkerComponent->ShowMarker();
}

void ULockOnComponent::HideTargetMarker(AActor* TargetActor) const
{
	if (nullptr == TargetActor)
	{
		return;
	}
	
	ULockOnMarkerComponent* MarkerComponent = TargetActor->FindComponentByClass<ULockOnMarkerComponent>();
	
	if (nullptr == MarkerComponent)
	{
		return;
	}
	
	MarkerComponent->HideMarker();
}

