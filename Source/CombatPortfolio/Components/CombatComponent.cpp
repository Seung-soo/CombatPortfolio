// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "HealthComponent.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	ComboSectionNames.Add(TEXT("Attack_1"));
	ComboSectionNames.Add(TEXT("Attack_2"));
	ComboSectionNames.Add(TEXT("Attack_3"));
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
	if (CombatActionState == ECombatActionState::Idle)
	{
		return StartAttack();
	}
	
	if (CombatActionState == ECombatActionState::Attacking)
	{
		return TryBufferComboInput();
	}
	
	return false;
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

bool UCombatComponent::IsComboInputWindowOpen() const
{
	return bComboInputWindowOpen;
}

bool UCombatComponent::HasBufferedComboInput() const
{
	return bComboInputBuffered;
}

int UCombatComponent::GetCurrentComboIndex() const
{
	return CurrentComboIndex;
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

void UCombatComponent::BeginComboInputWindow()
{
	if (false == IsAttacking())
	{
		return;
	}
	
	if (false == CanMoveToNextCombo())
	{
		return;
	}
	
	SetComboInputWindowOpen(true);
}

void UCombatComponent::EndComboInputWindow()
{
	SetComboInputWindowOpen(false);
	TryCommitBufferedCombo();
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
	
	ResetComboState();
	
	const float MontageDuration = AnimInstance->Montage_Play(AttackMontage, AttackPlayRate);
	
	if (0.0f >= MontageDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed: Montage_Play returned 0."));
		return false;
	}
	
	const FName FirstSectionName = GetCurrentComboSectionName();
	
	if (NAME_None != FirstSectionName)
	{
		AnimInstance->Montage_JumpToSection(FirstSectionName, AttackMontage);
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UCombatComponent::HandleAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AttackMontage);
	
	SetCombatActionState(ECombatActionState::Attacking);
	
	return true;
}

bool UCombatComponent::TryBufferComboInput()
{
	if (false == IsComboInputWindowOpen())
	{
		return false;
	}
	
	if (false == CanMoveToNextCombo())
	{
		return false;
	}
	
	SetComboInputBuffered(true);
	
	UE_LOG(LogTemp, Log, TEXT("Combo input buffered. CurrentComboIndex: %d"), CurrentComboIndex);
	
	return true;
}

bool UCombatComponent::TryCommitBufferedCombo()
{
	if (false == HasBufferedComboInput())
	{
		return false;
	}
	
	if (false == CanMoveToNextCombo())
	{
		SetComboInputBuffered(false);
		return false;
	}
	
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		SetComboInputBuffered(false);
		return false;
	}
	
	CurrentComboIndex++;
	
	const FName NextSectionName = GetCurrentComboSectionName();
	
	if (NAME_None == NextSectionName)
	{
		SetComboInputBuffered(false);
		return false;
	}
	
	HitActorsThisAttack.Reset();
	
	SetHitWindowOpen(false);
	SetComponentTickEnabled(false);
	SetComboInputWindowOpen(false);
	SetComboInputBuffered(false);
	
	AnimInstance->Montage_JumpToSection(NextSectionName, AttackMontage);
	
	UE_LOG(LogTemp, Log, TEXT("Combo committed. Jump to section: %s"), *NextSectionName.ToString());
	
	OnComboStateChanged.Broadcast();
	
	return true;
}

bool UCombatComponent::CanMoveToNextCombo() const
{
	return ComboSectionNames.IsValidIndex(CurrentComboIndex + 1);
}

FName UCombatComponent::GetCurrentComboSectionName() const
{
	if (false == ComboSectionNames.IsValidIndex(CurrentComboIndex))
	{
		return NAME_None;
	}
	
	return ComboSectionNames[CurrentComboIndex];
}

FName UCombatComponent::GetNextComboSectionName() const
{
	if (false == ComboSectionNames.IsValidIndex(CurrentComboIndex + 1))
	{
		return NAME_None;
	}
	
	return ComboSectionNames[CurrentComboIndex + 1];
}

void UCombatComponent::FinishAttack()
{
	SetHitWindowOpen(false);
	SetComboInputWindowOpen(false);
	SetComboInputBuffered(false);
	SetComponentTickEnabled(false);
	HitActorsThisAttack.Reset();
	
	ResetComboState();
	
	SetCombatActionState(ECombatActionState::Idle);
}

void UCombatComponent::ResetComboState()
{
	CurrentComboIndex = 0;
	bComboInputWindowOpen = false;
	bComboInputBuffered = false;
	
	OnComboStateChanged.Broadcast();
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

void UCombatComponent::SetComboInputWindowOpen(bool bNewComboInputWindowOpen)
{
	if (bComboInputWindowOpen == bNewComboInputWindowOpen)
	{
		return;
	}
	
	bComboInputWindowOpen = bNewComboInputWindowOpen;
	
	OnComboStateChanged.Broadcast();
}

void UCombatComponent::SetComboInputBuffered(bool bNewComboInputBuffered)
{
	if (bComboInputBuffered == bNewComboInputBuffered)
	{
		return;
	}
	
	bComboInputBuffered = bNewComboInputBuffered;
	
	OnComboStateChanged.Broadcast();
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
		
		ApplyDamageToHitActor(HitActor);
	}
}

void UCombatComponent::ApplyDamageToHitActor(AActor* HitActor)
{
	if (nullptr == HitActor)
	{
		return;
	}
	
	UHealthComponent* HealthComponent = HitActor->FindComponentByClass<UHealthComponent>();
	
	if (nullptr == HealthComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("Hit Actor has no HealthComponent: %s"), *HitActor->GetName());
		return;
	}
	
	const bool bDamageApplied = HealthComponent->ApplyDamage(AttackDamage);
	
	if (false == bDamageApplied)
	{
		UE_LOG(LogTemp, Log, TEXT("Damage was not applied to: %s"), *HitActor->GetName());
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Applied %.1f damage to %s. Current HP: %.1f, / %.1f"), 
		AttackDamage, *HitActor->GetName(), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
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
