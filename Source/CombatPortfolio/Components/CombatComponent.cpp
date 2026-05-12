// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "HealthComponent.h"
#include "CombatPortfolio/Combat/CombatDamageLibrary.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	FComboAttackData FirstAttackData;
	FirstAttackData.SectionName = TEXT("Attack_1");
	FirstAttackData.Damage = 20.0f;
	FirstAttackData.TraceRadius = 75.0f;
	FirstAttackData.TraceForwardOffset = 130.0f;
	FirstAttackData.TraceHalfHeight = 40.0f;
	
	FComboAttackData SecondAttackData;
	SecondAttackData.SectionName = TEXT("Attack_2");
	SecondAttackData.Damage = 25.0f;
	SecondAttackData.TraceRadius = 85.0f;
	SecondAttackData.TraceForwardOffset = 145.0f;
	SecondAttackData.TraceHalfHeight = 55.0f;
	
	FComboAttackData ThirdAttackData;
	ThirdAttackData.SectionName = TEXT("Attack_3");
	ThirdAttackData.Damage = 40.0f;
	ThirdAttackData.TraceRadius = 110.0f;
	ThirdAttackData.TraceForwardOffset = 170.0f;
	ThirdAttackData.TraceHalfHeight = 55.0f;
	
	ComboAttackDataList.Add(FirstAttackData);
	ComboAttackDataList.Add(SecondAttackData);
	ComboAttackDataList.Add(ThirdAttackData);
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

void UCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();

	if (World != nullptr)
	{
		World->GetTimerManager().ClearTimer(HitReactionInvincibleTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
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

bool UCombatComponent::RequestDodge(const FVector& DodgeDirection)
{
	if (false == CanStartDodge())
	{
		return false;
	}
	
	return StartDodge(DodgeDirection);
}

bool UCombatComponent::RequestDeath()
{
	if (ECombatActionState::Dead == CombatActionState)
	{
		return false;
	}
	
	CancelCurrentActionForInterrupt();
	
	bDeathMontageFinished = false;
	CurrentHitReactionMontage = nullptr;
	
	SetCombatActionState(ECombatActionState::Dead);
	
	SetDodgeInvincible(false);
	SetHitReactionInvincible(false);
	
	const bool bDeathMontageStarted = TryPlayDeathMontage();
	if (false == bDeathMontageStarted)
	{
		UE_LOG(LogTemp, Error, TEXT("DeathMontage is not assigned or failed to play. Assign AM_Player_Death to CombatComponent"));
	}
	
	return true;
}

bool UCombatComponent::CanStartDodge() const
{
	return ECombatActionState::Idle == CombatActionState;
}

bool UCombatComponent::IsDodging() const
{
	return ECombatActionState::Dodging == CombatActionState;
}

bool UCombatComponent::IsDead() const
{
	return ECombatActionState::Dead == CombatActionState;
}

bool UCombatComponent::IsInvincible() const
{
	return bHitReactionInvincible || bDodgeInvincible;
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

float UCombatComponent::GetCurrentAttackDamage() const
{
	const FComboAttackData* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
	{
		return 0.0f;
	}
	
	return CurrentAttackData->Damage;
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

bool UCombatComponent::RequestHitReaction(const FCombatDamageInfo& DamageInfo)
{
	if (ECombatActionState::Dead == CombatActionState)
	{
		return false;
	}
	
	if (ECombatActionState::HitReaction == CombatActionState)
	{
		return false;
	}
	
	if (ECombatActionState::Dodging == CombatActionState)
	{
		return false;
	}
	
	CancelCurrentActionForInterrupt();
	
	SetCombatActionState(ECombatActionState::HitReaction);
	
	StartHitReactionInvincibility();
	
	const bool bPlayedMontage = TryPlayHitReactionMontage(DamageInfo);
	
	if (false == bPlayedMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("HitReactionMontage failed. Strength: %s, Direction: %s"),
			GetCombatHitStrengthDebugString(DamageInfo.HitStrength),
			GetCombatHitDirectionDebugString(DamageInfo.HitDirectionType));
		
		CurrentHitReactionMontage = nullptr;
		EndHitReaction();
	}

	return true;
}

bool UCombatComponent::IsHitReacting() const
{
	return ECombatActionState::HitReaction == CombatActionState;
}

bool UCombatComponent::IsDeathMontageFinished() const
{
	return bDeathMontageFinished;
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
	
	if (0 >= ComboAttackDataList.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed: ComboAttackDataList is empty"));
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
	
	UE_LOG(LogTemp, Log, TEXT("Attack started. Section: %s, Damage: %.1f"), *FirstSectionName.ToString(), GetCurrentAttackDamage());
	
	return true;
}

bool UCombatComponent::StartDodge(const FVector& DodgeDirection)
{
	ResetComboState();
	
	SetHitWindowOpen(false);
	SetComboInputWindowOpen(false);
	SetComboInputBuffered(false);
	SetComponentTickEnabled(false);
	HitActorsThisAttack.Reset();
	
	SetCombatActionState(ECombatActionState::Dodging);
	
	BeginDodgeInvincibility();
	
	ApplyDodgeMovement(DodgeDirection);
	
	const bool bDodgeMontagePlayed = TryPlayDodgeMontage();
	
	UWorld* World = GetWorld();
	
	if (nullptr != World)
	{
		World->GetTimerManager().ClearTimer(DodgeFallbackTimerHandle);
		
		World->GetTimerManager().SetTimer(
			DodgeFallbackTimerHandle,
			this,
			&UCombatComponent::FinishDodge,
			DodgeDuration,
			false
		);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Dodge started. MontagePlayed: %s"), true == bDodgeMontagePlayed ? TEXT("true") : TEXT("false"));
	
	return true;
}

bool UCombatComponent::TryPlayDodgeMontage()
{
	if (nullptr == DodgeMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("DodgeMontage is not assigned. Dodge movement will still happen"));
		return false;
	}
	
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		return false;
	}
	
	const float MontageDuration = AnimInstance->Montage_Play(DodgeMontage, DodgePlayRate);
	
	if (0.0f >= MontageDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dodge Montage_Play failed."));
		return false;
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UCombatComponent::HandleDodgeMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DodgeMontage);
	
	return true;
}

void UCombatComponent::ApplyDodgeMovement(const FVector& DodgeDirection)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr == OwnerCharacter)
	{
		return;
	}
	
	FVector SafeDodgeDirection = DodgeDirection.GetSafeNormal();
	
	if (true == SafeDodgeDirection.IsNearlyZero())
	{
		SafeDodgeDirection = OwnerCharacter->GetActorForwardVector();
	}
	
	OwnerCharacter->LaunchCharacter(SafeDodgeDirection * DodgeStrength, true, false);
}

void UCombatComponent::BeginDodgeInvincibility()
{
	SetDodgeInvincible(true);
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
	
	World->GetTimerManager().SetTimer(
		InvincibilityTimerHandle,
		this,
		&UCombatComponent::EndDodgeInvincibility,
		DodgeInvincibleDuration,
		false
	);
}

void UCombatComponent::EndDodgeInvincibility()
{
	SetDodgeInvincible(false);
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
	
	UE_LOG(LogTemp, Log, TEXT("Combo committed. Jump to section: %s, Damage: %.1f"), *NextSectionName.ToString(), GetCurrentAttackDamage());
	
	OnComboStateChanged.Broadcast();
	
	return true;
}

bool UCombatComponent::CanMoveToNextCombo() const
{
	return ComboAttackDataList.IsValidIndex(CurrentComboIndex + 1);
}

const FComboAttackData* UCombatComponent::GetCurrentComboAttackData() const
{
	return GetComboAttackDataByIndex(CurrentComboIndex);
}

const FComboAttackData* UCombatComponent::GetComboAttackDataByIndex(int32 ComboIndex) const
{
	if (false == ComboAttackDataList.IsValidIndex(ComboIndex))
	{
		return nullptr;
	}
	
	return &ComboAttackDataList[ComboIndex];
}

FName UCombatComponent::GetCurrentComboSectionName() const
{
	const FComboAttackData* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
	{
		return NAME_None;
	}

	return CurrentAttackData->SectionName;
}

FName UCombatComponent::GetNextComboSectionName() const
{
	const FComboAttackData* NextAttackData = GetComboAttackDataByIndex(CurrentComboIndex + 1);
	
	if (nullptr == NextAttackData)
	{
		return NAME_None;
	}
	
	return NextAttackData->SectionName;
}

void UCombatComponent::FinishAttack()
{
	if (ECombatActionState::Attacking != CombatActionState)
	{
		return;
	}
	
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

void UCombatComponent::HandleDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != DodgeMontage)
	{
		return;
	}
	
	FinishDodge();
}

void UCombatComponent::FinishDodge()
{
	if (false == IsDodging())
	{
		return;
	}
	
	UWorld* World = GetWorld();
	
	if (nullptr != World)
	{
		World->GetTimerManager().ClearTimer(DodgeFallbackTimerHandle);
	}
	
	EndDodgeInvincibility();
	
	SetCombatActionState(ECombatActionState::Idle);
	
	UE_LOG(LogTemp, Log, TEXT("Dodge finished."));
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

void UCombatComponent::SetDodgeInvincible(bool bNewInvincible)
{
	const bool bWasInvincible = IsInvincible();
	
	if (bDodgeInvincible == bNewInvincible)
	{
		return;
	}
	
	bDodgeInvincible = bNewInvincible;
	
	if (bWasInvincible != IsInvincible())
	{
		OnInvincibilityChanged.Broadcast();
	}
}

void UCombatComponent::SetHitReactionInvincible(bool bNewInvincible)
{
	const bool bWasInvincible = IsInvincible();
	
	if (bHitReactionInvincible == bNewInvincible)
	{
		return;
	}
	
	bHitReactionInvincible = bNewInvincible;
	
	if (bWasInvincible != IsInvincible())
	{
		OnInvincibilityChanged.Broadcast();
	}
}

void UCombatComponent::BroadcastInvincibilityIfChanged(bool bOldInvincible)
{
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
	
	const FComboAttackData* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
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
		FCollisionShape::MakeSphere(CurrentAttackData->TraceRadius),
		QueryParams
	);
	
	if (true == bDrawAttackTraceDebug)
	{
		const FColor DebugColor = true == bHit ? FColor::Red : FColor::Green;
		
		const FVector Center = (StartLocation + EndLocation) * 0.5f;
		const float CapsuleHalfHeight = (EndLocation - StartLocation).Size() * 0.5f + CurrentAttackData->TraceRadius;
		
		DrawDebugCapsule(
			World,
			Center,
			CapsuleHalfHeight,
			CurrentAttackData->TraceRadius,
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
		
		if (true == HasAlreadyHitActor(HitActor))
		{
			continue;
		}
		
		RegisterHitActor(HitActor);
		
		UE_LOG(LogTemp, Log, TEXT("Attack Trace Hit Actor: %s"), *HitActor->GetName());
		
		ApplyDamageToHitActor(HitResult);
	}
}

void UCombatComponent::ApplyDamageToHitActor(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	
	if (nullptr == HitActor)
	{
		return;
	}
	
	const FComboAttackData* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
	{
		return;
	}
	
	if (true == IsDamageBlockedByInvincibility(HitActor))
	{
		UE_LOG(LogTemp, Log, TEXT("Damage blocked by invincibility: %s"), *HitActor->GetName());
		return;
	}
	
	UHealthComponent* HealthComponent = HitActor->FindComponentByClass<UHealthComponent>();
	
	if (nullptr == HealthComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("Hit Actor has no HealthComponent: %s"), *HitActor->GetName());
		return;
	}
	
	AActor* OwnerActor = GetOwner();
	
	FCombatDamageInfo DamageInfo;
	DamageInfo.DamageAmount = CurrentAttackData->Damage;
	DamageInfo.InstigatorActor = OwnerActor;
	DamageInfo.DamageCauser = OwnerActor;
	DamageInfo.HitActor = HitActor;
	DamageInfo.HitLocation = HitResult.ImpactPoint;
	DamageInfo.HitNormal = HitResult.ImpactNormal;
	DamageInfo.HitDirection = nullptr != OwnerActor ? 
	(HitActor->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal() : FVector::ZeroVector;
	DamageInfo.HitStrength = CurrentAttackData->HitStrength;
	DamageInfo.HitDirectionType = UCombatDamageLibrary::CalculateHitDirectionFromIncomingDirection(HitActor, DamageInfo.HitDirection);
	DamageInfo.KnockbackStrength = CurrentAttackData->KnockbackStrength;
	DamageInfo.HitStopDuration = CurrentAttackData->HitStopDuration;
	DamageInfo.HitStopTimeDilation = CurrentAttackData->HitStopTimeDilation;
	
	const bool bDamageApplied = HealthComponent->ApplyDamage(DamageInfo);
	
	if (false == bDamageApplied)
	{
		UE_LOG(LogTemp, Log, TEXT("Damage was not applied to: %s"), *HitActor->GetName());
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Applied %.1f damage to %s. Current HP: %.1f, / %.1f"), 
		DamageInfo.DamageAmount, *HitActor->GetName(), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
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
	
	const FComboAttackData* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
	{
		return FVector::ZeroVector;
	}
	
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector ForwardVector = OwnerActor->GetActorForwardVector();
	
	return OwnerLocation + ForwardVector * CurrentAttackData->TraceForwardOffset - FVector(0.0f, 0.0f, CurrentAttackData->TraceHalfHeight);
}

FVector UCombatComponent::GetAttackTraceEndLocation() const
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return FVector::ZeroVector;
	}
	
	const FComboAttackData* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
	{
		return FVector::ZeroVector;
	}
	
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector ForwardVector = OwnerActor->GetActorForwardVector();
	
	return OwnerLocation + ForwardVector * CurrentAttackData->TraceForwardOffset + FVector(0.0f, 0.0f, CurrentAttackData->TraceHalfHeight);
}

bool UCombatComponent::IsDamageBlockedByInvincibility(const AActor* TargetActor) const
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

void UCombatComponent::EndHitReaction()
{
	if (ECombatActionState::HitReaction != CombatActionState)
	{
		return;
	}
	
	CurrentHitReactionMontage = nullptr;
	
	SetCombatActionState(ECombatActionState::Idle);
	
	UE_LOG(LogTemp, Log, TEXT("Player hit reaction ended."));
}

bool UCombatComponent::TryPlayHitReactionMontage(const FCombatDamageInfo& DamageInfo)
{
	UAnimMontage* HitReactionMontage = GetHitReactionMontageByDirection(DamageInfo.HitDirectionType);
	
	if (nullptr == HitReactionMontage)
	{
		CurrentHitReactionMontage = nullptr;
		return false;
	}
	
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		CurrentHitReactionMontage = nullptr;
		return false;
	}
	
	const float MontageLength = AnimInstance->Montage_Play(HitReactionMontage, HitReactionMontagePlayRate);
	
	if (0.0f >= MontageLength)
	{
		CurrentHitReactionMontage = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("HitReactionMontage failed to play. Direction: %s"),
			GetCombatHitStrengthDebugString(DamageInfo.HitStrength));
		
		return false;
	}
	
	CurrentHitReactionMontage = HitReactionMontage;
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UCombatComponent::HandleHitReactionMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, HitReactionMontage);
	
	return true;
}

UAnimMontage* UCombatComponent::GetHitReactionMontageByDirection(ECombatHitDirection HitDirection) const
{
	UAnimMontage* SelectedMontage = nullptr;
	
	switch (HitDirection)
	{
	case ECombatHitDirection::Front:
		SelectedMontage = FrontHitReactionMontage;
		break;
	case ECombatHitDirection::Back:
		SelectedMontage = BackHitReactionMontage;
		break;
	case ECombatHitDirection::Left:
		SelectedMontage = LeftHitReactionMontage;
		break;
	case ECombatHitDirection::Right:
		SelectedMontage = RightHitReactionMontage;
		break;
	default:
		break;
	}
	
	return nullptr != SelectedMontage ? SelectedMontage : FrontHitReactionMontage.Get();
}

void UCombatComponent::StartHitReactionInvincibility()
{
	if (0.0f >= HitReactionInvincibleDuration)
	{
		SetHitReactionInvincible(false);
		return;
	}
	
	SetHitReactionInvincible(true);
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(HitReactionInvincibleTimerHandle);
	
	World->GetTimerManager().SetTimer(
		HitReactionInvincibleTimerHandle,
		this,
		&UCombatComponent::EndHitReactionInvincibility,
		HitReactionInvincibleDuration,
		false
	);
}

void UCombatComponent::EndHitReactionInvincibility()
{
	SetHitReactionInvincible(false);
}

void UCombatComponent::HandleHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentHitReactionMontage)
	{
		return;
	}
	
	CurrentHitReactionMontage = nullptr;
	
	EndHitReaction();
}

bool UCombatComponent::TryPlayDeathMontage()
{
	if (nullptr == DeathMontage)
	{
		return false;
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr == OwnerCharacter)
	{
		return false;
	}
	
	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	
	if (nullptr == OwnerMesh)
	{
		return false;
	}
	
	UAnimInstance* AnimInstance = OwnerMesh->GetAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		return false;
	}
	
	const float MontageLength = OwnerCharacter->PlayAnimMontage(DeathMontage, DeathMontagePlayRate);
	
	if (0.0f >= MontageLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to play DeathMontage."), *OwnerCharacter->GetName());
		return false;
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UCombatComponent::HandleDeathMontageEnded);
	
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DeathMontage);
	
	return true;
}

void UCombatComponent::HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != DeathMontage)
	{
		return;
	}
	
	bDeathMontageFinished = true;
	
	UE_LOG(LogTemp, Log, TEXT("Player death montage ended. Interrupted: %s"), bInterrupted ? TEXT("true") : TEXT("false"));
}

void UCombatComponent::CancelCurrentActionForInterrupt()
{
	switch (CombatActionState)
	{
	case ECombatActionState::Attacking:
		CancelAttack();
		break;
	case ECombatActionState::Dodging:
		SetDodgeInvincible(false);
		break;
	case ECombatActionState::HitReaction:
		CurrentHitReactionMontage = nullptr;
		SetHitReactionInvincible(false);
		break;
	case ECombatActionState::Idle:
		break;
	case ECombatActionState::Dead:
		break;
	default:
		break;
	}
}

void UCombatComponent::CancelAttack()
{
	ResetAttackHitState();
	ResetComboState();
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr != OwnerCharacter)
	{
		OwnerCharacter->StopAnimMontage(AttackMontage);
	}
	
	if (ECombatActionState::Attacking == CombatActionState)
	{
		SetCombatActionState(ECombatActionState::Idle);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Player attack canceled"));
}

void UCombatComponent::ResetAttackHitState()
{
	SetHitWindowOpen(false);
	HitActorsThisAttack.Reset();
}
