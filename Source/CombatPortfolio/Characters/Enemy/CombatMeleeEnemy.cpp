// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatMeleeEnemy.h"

#include "AIController.h"
#include "CombatPortfolio/Components/EnemyAttackComponent.h"
#include "CombatPortfolio/Components/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"

ACombatMeleeEnemy::ACombatMeleeEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	if (nullptr != EnemyAttackComponent)
	{
		EnemyAttackComponent->SetStartAttackOnBeginPlay(false);
	}
		
	if (nullptr != BodyMeshComponent)
	{
		BodyMeshComponent->SetVisibility(false);
	}
}

void ACombatMeleeEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	CachePlayerPawn();
	SetMeleeEnemyState(EMeleeEnemyState::Idle);
	
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	
	if (nullptr != MovementComponent)
	{
		MovementComponent->MaxWalkSpeed = ChaseMoveSpeed;
	}
}

void ACombatMeleeEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (nullptr != HealthComponent && true == HealthComponent->IsDead())
	{
		SetMeleeEnemyState(EMeleeEnemyState::Dead);
		StopChaseMovement();
		return;
	}
	
	if (true == IsHitReacting())
	{
		SetMeleeEnemyState(EMeleeEnemyState::HitReacting);
		StopChaseMovement();
		return;
	}
	
	if (false == HasValidTarget())
	{
		CachePlayerPawn();
		SetMeleeEnemyState(EMeleeEnemyState::Idle);
		StopChaseMovement();
		return;
	}
	
	if (true == IsTargetDead())
	{
		SetMeleeEnemyState(EMeleeEnemyState::Idle);
		StopChaseMovement();
		return;
	}
	
	if (true == bDrawMeleeDebug)
	{
		DrawMeleeDebug();
	}
	
	if (false == IsTargetInsideDetectionRadius())
	{
		SetMeleeEnemyState(EMeleeEnemyState::Idle);
		StopChaseMovement();
		return;
	}
	
	UpdateFacingToTarget(DeltaTime);
	
	if (nullptr != EnemyAttackComponent && true == EnemyAttackComponent->IsAttacking())
	{
		SetMeleeEnemyState(EMeleeEnemyState::Attacking);
		StopChaseMovement();
		return;
	}
	
	if (false == IsTargetInsideStopDistance())
	{
		SetMeleeEnemyState(EMeleeEnemyState::Chasing);
		UpdateChaseMovement();
		return;
	}
	
	StopChaseMovement();
	
	if (false == IsTargetInsideAttackRange())
	{
		SetMeleeEnemyState(EMeleeEnemyState::Idle);
		return;
	}
	
	if (false == IsFacingTarget())
	{
		SetMeleeEnemyState(EMeleeEnemyState::Idle);
		return;
	}
	
	TryAttackTarget();
}

void ACombatMeleeEnemy::ApplyDeathState()
{
	Super::ApplyDeathState();
	
	StopChaseMovement();
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		World->GetTimerManager().ClearTimer(HitReactionTimerHandle);
	}
	
	bHitReacting = false;
	
	SetMeleeEnemyState(EMeleeEnemyState::Dead);
	SetActorTickEnabled(false);
	TargetPlayerPawn.Reset();
}

void ACombatMeleeEnemy::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
	Super::HandleHealthChanged(CurrentHealth, MaxHealth, Delta);
	
	if (0.0f >= CurrentHealth)
	{
		return;
	}
	
	if (0.0f > Delta)
	{
		StartHitReaction();
	}
}

void ACombatMeleeEnemy::CachePlayerPawn()
{
	TargetPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
}

bool ACombatMeleeEnemy::HasValidTarget() const
{
	return TargetPlayerPawn.IsValid();
}

bool ACombatMeleeEnemy::IsTargetDead() const
{
	if (const APawn* TargetPawn = TargetPlayerPawn.Get())
	{
		if (nullptr == TargetPawn)
		{
			return true;
		}
	}
	
	UHealthComponent* TargetHealthComponent = TargetPlayerPawn->FindComponentByClass<UHealthComponent>();
	
	if (nullptr == TargetHealthComponent)
	{
		return false;
	}
	
	return TargetHealthComponent->IsDead();
}

float ACombatMeleeEnemy::GetDistanceToTarget() const
{
	const APawn* TargetPawn = TargetPlayerPawn.Get();
	
	if (nullptr == TargetPawn)
	{
		return TNumericLimits<float>::Max();
	}
	
	return FVector::Dist2D(GetActorLocation(), TargetPawn->GetActorLocation());
}

bool ACombatMeleeEnemy::IsTargetInsideDetectionRadius() const
{
	return GetDistanceToTarget() <= DetectionRadius;
}

bool ACombatMeleeEnemy::IsTargetInsideAttackRange() const
{
	return GetDistanceToTarget() <= AttackRange;
}

bool ACombatMeleeEnemy::IsTargetInsideStopDistance() const
{
	return GetDistanceToTarget() <= StopDistance;
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

void ACombatMeleeEnemy::UpdateChaseMovement()
{
	APawn* TargetPawn = TargetPlayerPawn.Get();
	
	if (nullptr == TargetPawn)
	{
		return;
	}
	
	AAIController* EnemyAIController = Cast<AAIController>(GetController());
	
	if (nullptr == EnemyAIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no AIController."), *GetName());
		return;
	}
	
	EnemyAIController->MoveToActor(TargetPawn, NavigationAcceptanceRadius, false, true, true, nullptr, true);
}

void ACombatMeleeEnemy::StopChaseMovement()
{
	AAIController* EnemyAIController = Cast<AAIController>(GetController());
	
	if (nullptr == EnemyAIController)
	{
		return;
	}
	
	EnemyAIController->StopMovement();
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
		SetMeleeEnemyState(EMeleeEnemyState::Attacking);
		UE_LOG(LogTemp, Log, TEXT("%s requested melee attack."), *GetName());
	}
}

void ACombatMeleeEnemy::StartHitReaction()
{
	if (nullptr != HealthComponent && true == HealthComponent->IsDead())
	{
		return;
	}
	
	bHitReacting = true;
	SetMeleeEnemyState(EMeleeEnemyState::HitReacting);
	
	StopChaseMovement();
	
	if (nullptr != EnemyAttackComponent)
	{
		EnemyAttackComponent->CancelAttack();
	}
	
	const bool bPlayedMontage = TryPlayHitReactionMontage();
	
	float ReactionDuration = HitReactionDuration;
	
	if (true == bPlayedMontage && nullptr != HitReactionMontage)
	{
		const float MontageDuration = HitReactionMontage->GetPlayLength() / FMath::Max(HitReactionMontagePlayRate, KINDA_SMALL_NUMBER);
		ReactionDuration = FMath::Max(ReactionDuration, MontageDuration);
	}
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(HitReactionTimerHandle);
	World->GetTimerManager().SetTimer(
		HitReactionTimerHandle,
		this,
		&ACombatMeleeEnemy::EndHitReaction,
		ReactionDuration,
		false
	);
}

void ACombatMeleeEnemy::EndHitReaction()
{
	if (nullptr != HealthComponent && true == HealthComponent->IsDead())
	{
		return;
	}
	
	bHitReacting = false;
	SetMeleeEnemyState(EMeleeEnemyState::Idle);
	
	UE_LOG(LogTemp, Log, TEXT("%s hit reaction ended."), *GetName());
}

bool ACombatMeleeEnemy::IsHitReacting() const
{
	return bHitReacting;
}

bool ACombatMeleeEnemy::TryPlayHitReactionMontage()
{
	if (nullptr == HitReactionMontage)
	{
		return false;
	}
	
	const float MontageLength = PlayAnimMontage(HitReactionMontage, HitReactionMontagePlayRate);
	
	if (0.0f >= MontageLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to play hit reaction montage."), *GetName());
		return false;
	}
	
	return true;
}

void ACombatMeleeEnemy::SetMeleeEnemyState(EMeleeEnemyState NewState)
{
	if (MeleeEnemyState == NewState)
	{
		return;
	}
	
	MeleeEnemyState = NewState;
	
	UE_LOG(LogTemp, Log, TEXT("%s, MeleeState: %s"), *GetName(), *GetMeleeEnemyStateDebugString());
}

FString ACombatMeleeEnemy::GetMeleeEnemyStateDebugString() const
{
	switch (MeleeEnemyState)
	{
	case EMeleeEnemyState::Idle:
		return TEXT("Idle");
	case EMeleeEnemyState::Chasing:
		return TEXT("Chasing");
	case EMeleeEnemyState::Attacking:
		return TEXT("Attacking");
	case EMeleeEnemyState::HitReacting:
		return TEXT("HitReacting");
	case EMeleeEnemyState::Dead:
		return TEXT("Dead");
	default:
		return TEXT("Unknown");
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
	
	DrawDebugSphere(World, ActorLocation, StopDistance, 32, FColor::Green, false, 0.0f);
	
	if (true == HasValidTarget())
	{
		const FVector DirectionToTarget = GetPlanarDirectionToTarget();
		
		DrawDebugDirectionalArrow(
			World,
			ActorLocation + FVector(0.0f, 0.0f, 80.0f),
			ActorLocation + FVector(0.0f, 0.0f, 80.0f) + DirectionToTarget * 200.0f,
			50.0f,
			FColor::Cyan,
			false,
			0.0f,
			0,
			3.0f
		);
	}
}
