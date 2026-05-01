// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetComponentTickEnabled(false);
}

void UCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (false == IsHitWindowOpen())
	{
		return;
	}
	
	// 충돌 검사를 할 때만 Tick이 켜지기 때문에 이런 구조가 가능
	PerformAttackTrace();
}

bool UCombatComponent::RequestAttack()
{
	if (false == CanStartAttack())
	{
		return false;
	}
	
	return StartAttack();
}

bool UCombatComponent::CanStartAttack() const
{
	return ECombatActionState::Idle == CombatActionState;
}

bool UCombatComponent::IsAttacking() const
{
	return ECombatActionState::Attacking == CombatActionState;
}

bool UCombatComponent::IsHitWindowOpen() const
{
	return bHitWindowOpen;
}

int32 UCombatComponent::GetHitActorCountThisAttack() const
{
	return HitActorsThisAttack.Num();
}

void UCombatComponent::BeginHitWindow()
{
	if (false == IsAttacking())
	{
		return;
	}
	
	HitActorsThisAttack.Reset();
	
	SetHitWindowOpen(true);
	SetComponentTickEnabled(true);
}

void UCombatComponent::EndHitWindow()
{
	SetHitWindowOpen(false);
	SetComponentTickEnabled(false);
}

ECombatActionState UCombatComponent::GetCombatActionState() const
{
	return CombatActionState;
}

bool UCombatComponent::StartAttack()
{
	if (nullptr == AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed: AttackMontage is not assigned."));
		return false;
	}
	
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed: AnimInstance is not valid."));
		return false;
	}
	
	const float MontageDuration = AnimInstance->Montage_Play(AttackMontage, AttackPlayRate);
	
	if (0.0f >= MontageDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed: Montage_Play returned 0."));
		return false;
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UCombatComponent::HandleAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AttackMontage);
	
	SetCombatActionState(ECombatActionState::Attacking);
	
	return true;
}

void UCombatComponent::FinishAttack()
{
	SetHitWindowOpen(false);
	SetComponentTickEnabled(false);
	HitActorsThisAttack.Reset();
	
	SetCombatActionState(ECombatActionState::Idle);
}

UAnimInstance* UCombatComponent::GetOwnerAnimInstance() const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr == OwnerCharacter)
	{
		return nullptr;
	}
	
	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	
	if (nullptr == OwnerMesh)
	{
		return nullptr;
	}
	
	return OwnerMesh->GetAnimInstance();
}

void UCombatComponent::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != AttackMontage)
	{
		return;
	}
	
	FinishAttack();
}

void UCombatComponent::SetCombatActionState(ECombatActionState NewCombatActionState)
{
	if (CombatActionState == NewCombatActionState)
	{
		return;
	}
	
	CombatActionState = NewCombatActionState;
	
	OnCombatActionStateChanged.Broadcast();
}

void UCombatComponent::SetHitWindowOpen(bool bNewHitWindowOpen)
{
	if (bHitWindowOpen == bNewHitWindowOpen)
	{
		return;
	}
	
	bHitWindowOpen = bNewHitWindowOpen;
	
	OnHitWindowChanged.Broadcast();
}

void UCombatComponent::PerformAttackTrace()
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
	
	if (true == bDrawAttackTraceDebug)
	{
		const FColor DebugColor = true == bHit ? FColor::Red : FColor::Green;
		
		const FVector Center = (StartLocation + EndLocation) * 0.5f;
		const float CapsuleHalfHeight = (EndLocation - StartLocation).Size() * 0.5f + AttackTraceRadius;
		
		DrawDebugCapsule(
			World,
			(StartLocation + EndLocation) * 0.5f,
			CapsuleHalfHeight,
			AttackTraceRadius,
			FQuat::Identity,
			DebugColor,
			false,
			0.05f
		);
	}
	
	if (false == bHit)
	{
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
		
		if (HasAlreadyHitActor(HitActor))
		{
			continue;
		}
		
		RegisterHitActor(HitActor);
		
		UE_LOG(LogTemp, Log, TEXT("Attack Trace Hit Actor: %s"), *HitActor->GetName());
	}
}

bool UCombatComponent::HasAlreadyHitActor(const AActor* HitActor) const
{
	if (nullptr == HitActor)
	{
		return false;
	}
	
	for (const TWeakObjectPtr<AActor>& HitActorPtr : HitActorsThisAttack)
	{
		if (HitActorPtr.Get() == HitActor)
		{
			return true;
		}
	}
	
	return false;
}

void UCombatComponent::RegisterHitActor(AActor* HitActor)
{
	if (nullptr == HitActor)
	{
		return;
	}
	
	HitActorsThisAttack.Add(HitActor);
}

FVector UCombatComponent::GetAttackTraceStartLocation() const
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

FVector UCombatComponent::GetAttackTraceEndLocation() const
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
