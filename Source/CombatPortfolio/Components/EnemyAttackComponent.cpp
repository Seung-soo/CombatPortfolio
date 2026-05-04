// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAttackComponent.h"

#include "CombatComponent.h"
#include "HealthComponent.h"

// Sets default values for this component's properties
UEnemyAttackComponent::UEnemyAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UEnemyAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (true == bStartAttackOnBeginPlay)
	{
		StartAutoAttack();
	}
}

void UEnemyAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAutoAttack();
	
	Super::EndPlay(EndPlayReason);
}

void UEnemyAttackComponent::StartAutoAttack()
{
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(AutoAttackTimerHandle);
	
	World->GetTimerManager().SetTimer(
		AutoAttackTimerHandle,
		this,
		&UEnemyAttackComponent::AttackOnce,
		AttackInterval,
		true
	);
}

void UEnemyAttackComponent::StopAutoAttack()
{
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(AutoAttackTimerHandle);
}

void UEnemyAttackComponent::AttackOnce()
{
	PerformAttackTrace();
}

void UEnemyAttackComponent::PerformAttackTrace()
{
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	const FVector StartLocation = GetAttackTraceStartLocation();
	const FVector EndLocation = GetAttackTraceEndLocation();
	
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	
	const bool bHit = World->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackTraceRadius),
		QueryParams
	);
	
	if (true == bDrawAttackDebug)
	{
		const FColor DebugColor = true == bHit ? FColor::Red : FColor::Blue;
		
		const FVector Center = (StartLocation + EndLocation) * 0.5f;
		const float CapsuleHalfHeight = (EndLocation - StartLocation).Size() * 0.5f + AttackTraceRadius;
		
		DrawDebugCapsule(
			World,
			Center,
			CapsuleHalfHeight,
			AttackTraceRadius,
			FQuat::Identity,
			DebugColor,
			false,
			0.25f
		);
	}
	
	if (false == bHit)
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy attack missed."));
		return;
	}
	
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		
		if (nullptr == HitActor)
		{
			continue;
		}
		
		if (HitActor == OwnerActor)
		{
			continue;
		}
		
		ApplyDamageToActor(HitActor);
	}
}

void UEnemyAttackComponent::ApplyDamageToActor(AActor* TargetActor)
{
	if (nullptr == TargetActor)
	{
		return;
	}
	
	if (true == IsDamageBlockedByInvincibility(TargetActor))
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy damage blocked by invincibility: %s"), *TargetActor->GetName());
		
		if (nullptr != GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Enemy Attack Blocked by IFrame"));
		}
		
		return;
	}
	
	UHealthComponent* HealthComponent = TargetActor->FindComponentByClass<UHealthComponent>();
	
	if (nullptr == HealthComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy hit actor has no HealthComponent: %s"), *TargetActor->GetName());
		return;
	}
	
	const bool bDamageApplied = HealthComponent->ApplyDamage(AttackDamage);
	
	if (false == bDamageApplied)
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy damage was not applied to: %s"), *TargetActor->GetName());
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Enemy applied %.1f damage to %s. HP: %.1f / %.1f"), AttackDamage, *TargetActor->GetName(), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
}

bool UEnemyAttackComponent::IsDamageBlockedByInvincibility(AActor* TargetActor) const
{
	if (nullptr == TargetActor)
	{
		return false;
	}
	
	const UCombatComponent* TargetCombatComponent = TargetActor->FindComponentByClass<UCombatComponent>();
	
	if (nullptr == TargetCombatComponent)
	{
		return false;
	}
	
	return TargetCombatComponent->IsInvincible();
}

FVector UEnemyAttackComponent::GetAttackTraceStartLocation() const
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return FVector::ZeroVector;
	}
	
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector ForwardVector = OwnerActor->GetActorForwardVector();
	
	return OwnerLocation + ForwardVector * AttackTraceForwardOffset - FVector(0.0f, 0.0f, AttackTraceHalfHeight);
}

FVector UEnemyAttackComponent::GetAttackTraceEndLocation() const
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return FVector::ZeroVector;
	}
	
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector ForwardVector = OwnerActor->GetActorForwardVector();
	
	return OwnerLocation + ForwardVector * AttackTraceForwardOffset + FVector(0.0f, 0.0f, AttackTraceHalfHeight);
}