// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatMeleeEnemy.h"

#include "CombatPortfolio/Components/EnemyAttackComponent.h"
#include "CombatPortfolio/Components/HealthComponent.h"
#include "Kismet/GameplayStatics.h"

ACombatMeleeEnemy::ACombatMeleeEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	if (nullptr != EnemyAttackComponent)
	{
		EnemyAttackComponent->SetStartAttackOnBeginPlay(false);
	}
}

void ACombatMeleeEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	CachePlayerPawn();
}

void ACombatMeleeEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (nullptr != HealthComponent && true == HealthComponent->IsDead())
	{
		return;
	}
	
	if (false == HasValidTarget())
	{
		CachePlayerPawn();
		return;
	}
	
	if (true == bDrawMeleeDebug)
	{
		DrawMeleeDebug();
	}
	
	if (false == IsTargetInsideDetectionRadius())
	{
		return;
	}
	
	UpdateFacingToTarget(DeltaTime);
	
	if (false == IsTargetInsideAttackRange())
	{
		return;
	}
	
	if (false == IsFacingTarget())
	{
		return;
	}
	
	TryAttackTarget();
}

void ACombatMeleeEnemy::ApplyDeathState()
{
	Super::ApplyDeathState();
	
	SetActorTickEnabled(false);
	TargetPlayerPawn.Reset();
}

void ACombatMeleeEnemy::CachePlayerPawn()
{
	TargetPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
}

bool ACombatMeleeEnemy::HasValidTarget() const
{
	return TargetPlayerPawn.IsValid();
}

float ACombatMeleeEnemy::GetDistanceToTarget() const
{
	const APawn* TargetPawn = TargetPlayerPawn.Get();
	
	if (nullptr == TargetPawn)
	{
		return TNumericLimits<float>::Max();
	}
	
	return FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
}

bool ACombatMeleeEnemy::IsTargetInsideDetectionRadius() const
{
	return GetDistanceToTarget() <= DetectionRadius;
}

bool ACombatMeleeEnemy::IsTargetInsideAttackRange() const
{
	return GetDistanceToTarget() <= AttackRange;
}

bool ACombatMeleeEnemy::IsFacingTarget() const
{
	const FVector DirectionToTarget = GetPlanarDirectionToTarget();
	
	if (true == DirectionToTarget.IsNearlyZero())
	{
		return false;
	}
	
	const FVector ForwardDirection = GetActorForwardVector().GetSafeNormal2D();
	
	const float Dot = FVector::DotProduct(ForwardDirection, DirectionToTarget);
	
	const float MinDot = FMath::Cos(FMath::DegreesToRadians(FacingAngleTolerance));
	
	return Dot >= MinDot;
}

FVector ACombatMeleeEnemy::GetPlanarDirectionToTarget() const
{
	const APawn* TargetPawn = TargetPlayerPawn.Get();
	
	if (nullptr == TargetPawn)
	{
		return FVector::ZeroVector;
	}
	
	FVector DirectionToTarget = TargetPawn->GetActorLocation() - GetActorLocation();
	DirectionToTarget.Z = 0.0f;
	
	return DirectionToTarget.GetSafeNormal();
}

void ACombatMeleeEnemy::UpdateFacingToTarget(float DeltaTime)
{
	const FVector DirectionToTarget = GetPlanarDirectionToTarget();
	
	if (true == DirectionToTarget.IsNearlyZero())
	{
		return;
	}
	
	const FRotator TargetRotation = DirectionToTarget.Rotation();
	const FRotator CurrentRotation = GetActorRotation();
	
	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		FRotator(0.0f, TargetRotation.Yaw, 0.0f),
		DeltaTime,
		FacingRotationInterpSpeed
	);
	
	SetActorRotation(NewRotation);
}

void ACombatMeleeEnemy::TryAttackTarget()
{
	if (nullptr == EnemyAttackComponent)
	{
		return;
	}
	
	const bool bAttackStarted = EnemyAttackComponent->RequestAttack();
	
	if (true == bAttackStarted)
	{
		UE_LOG(LogTemp, Log, TEXT("%s requested melee attack."), *GetName());
	}
}

void ACombatMeleeEnemy::DrawMeleeDebug() const
{
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	const FVector ActorLocation = GetActorLocation();
	
	DrawDebugSphere(World, ActorLocation, DetectionRadius, 32, FColor::Silver, false, 0.0f);
	
	DrawDebugSphere(World, ActorLocation, AttackRange, 32, FColor::Orange, false, 0.0f);
}
