// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttackComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "StaminaComponent.h"
#include "CombatPortfolio/CombatPortfolio.h"
#include "CombatPortfolio/Combat/CombatDamageLibrary.h"
#include "GameFramework/Character.h"

UPlayerAttackComponent::UPlayerAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


void UPlayerAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetComponentTickEnabled(false);
}

void UPlayerAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (false == IsHitWindowOpen())
	{
		return;
	}
	
	PerformAttackTrace();
}

bool UPlayerAttackComponent::StartAttack(ECombatAttackInputType AttackInputType)
{
	if (true == bAttackActive)
	{
		return false;
	}
	
	CurrentCombatAttackData = GetAttackDataByInputType(AttackInputType);
	CurrentAttackMontage = GetAttackMontageByInputType(AttackInputType);
	
	if (nullptr == CurrentCombatAttackData || 0 >= CurrentCombatAttackData->Attacks.Num())
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("PlayerAttack failed: AttackData is not assigned or empty"));
		return false;
	}
	
	if (nullptr == CurrentAttackMontage)
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("PlayerAttack failed: CurrentAttackMontage is not assigned."));
		return false;
	}
	
	ResetComboState();
	bCurrentAttackStaminaCostPaid = false;
	
	const FCombatAttackEntry* FirstAttackEntry = GetCurrentComboAttackData();
	
	if (nullptr == FirstAttackEntry)
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("PlayerAttack failed: FirstAttackEntry is null."));
		return false;
	}
	
	if (false == CanPayAttackStaminaCost(*FirstAttackEntry))
	{
		UE_LOG(LogCombatPortfolio, Log, TEXT("PlayerAttack failed: Not enough stamina. Required: %.1f"), FirstAttackEntry->StaminaCost);
		return false;
	}
	
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("PlayerAttack failed: AnimInstance is not valid."));
		return false;
	}
	
	bAttackActive = true;
	
	const float MontageDuration = AnimInstance->Montage_Play(CurrentAttackMontage, AttackPlayRate);
	
	if (0.0f >= MontageDuration)
	{
		bAttackActive = false;
		CurrentAttackMontage = nullptr;
		CurrentCombatAttackData = nullptr;
		
		UE_LOG(LogCombatPortfolio, Warning, TEXT("PlayerAttack failed: Montage_Play returned 0."));
		return false;
	}

	const FName FirstSectionName = GetCurrentComboSectionName();
	
	if (NAME_None != FirstSectionName)
	{
		AnimInstance->Montage_JumpToSection(FirstSectionName, CurrentAttackMontage);
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerAttackComponent::HandleAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, CurrentAttackMontage);
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Attack started. Section: %s, Damage: %.1f"), *FirstSectionName.ToString(), FirstAttackEntry->Damage);
	
	return true;
}

bool UPlayerAttackComponent::TryBufferComboInput()
{
	if (false == bAttackActive)
	{
		return false;
	}
	
	if (false == IsComboInputWindowOpen())
	{
		return false;
	}
	
	if (false == CanMoveToNextCombo())
	{
		return false;
	}
	
	SetComboInputBuffered(true);
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Combo input buffered. CurrentComboIndex: %d"), CurrentComboIndex);
	
	return true;
}

void UPlayerAttackComponent::CancelAttack()
{
	if (false == bAttackActive)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr != OwnerCharacter && nullptr != CurrentAttackMontage)
	{
		OwnerCharacter->StopAnimMontage(CurrentAttackMontage);
	}
	
	FinishAttack();
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("PlayerAttack canceled"));
}

void UPlayerAttackComponent::BeginHitWindow()
{
	if (false == bAttackActive)
	{
		return;
	}
	
	const FCombatAttackEntry* CurrentAttackEntry = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackEntry)
	{
		return;
	}
	
	if (false == bCurrentAttackStaminaCostPaid)
	{
		if (false == TrySpendAttackStaminaCost(*CurrentAttackEntry))
		{
			UE_LOG(LogCombatPortfolio, Log, TEXT("PlayerAttack HitWindow failed: Not enough stamina. Required: %.1f"), CurrentAttackEntry->StaminaCost);
			CancelAttack();
			return;
		}
		
		bCurrentAttackStaminaCostPaid = true;
		
		UE_LOG(LogCombatPortfolio, Log, TEXT("Attack stamina spent at HitWindow. Cost: %.1f"), CurrentAttackEntry->StaminaCost);
	}
	
	HitActorsThisAttack.Reset();
	
	SetHitWindowOpen(true);
	SetComponentTickEnabled(true);
}

void UPlayerAttackComponent::EndHitWindow()
{
	SetHitWindowOpen(false);
	SetComponentTickEnabled(false);
}

void UPlayerAttackComponent::BeginComboInputWindow()
{
	if (false == bAttackActive)
	{
		return;
	}
	
	if (false == CanMoveToNextCombo())
	{
		return;
	}
	
	SetComboInputWindowOpen(true);
}

void UPlayerAttackComponent::EndComboInputWindow()
{
	SetComboInputWindowOpen(false);
	TryCommitBufferedCombo();
}

bool UPlayerAttackComponent::IsAttacking() const
{
	return bAttackActive;
}

bool UPlayerAttackComponent::IsHitWindowOpen() const
{
	return bHitWindowOpen;
}

bool UPlayerAttackComponent::IsComboInputWindowOpen() const
{
	return bComboInputWindowOpen;
}

bool UPlayerAttackComponent::HasBufferedComboInput() const
{
	return bComboInputBuffered;
}

int UPlayerAttackComponent::GetCurrentComboIndex() const
{
	return CurrentComboIndex;
}

int32 UPlayerAttackComponent::GetHitActorCountThisAttack() const
{
	return HitActorsThisAttack.Num();
}

const FCombatAttackEntry* UPlayerAttackComponent::GetCurrentAttackEntry() const
{
	return GetCurrentComboAttackData();
}

void UPlayerAttackComponent::FinishAttack()
{
	if (false == bAttackActive)
	{
		return;
	}
	
	SetHitWindowOpen(false);
	SetComboInputWindowOpen(false);
	SetComboInputBuffered(false);
	SetComponentTickEnabled(false);
	
	HitActorsThisAttack.Reset();
	bCurrentAttackStaminaCostPaid = false;
	bAttackActive = false;
	
	CurrentAttackMontage = nullptr;
	CurrentCombatAttackData = nullptr;
	
	ResetComboState();
	
	OnAttackFinished.Broadcast();
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("PlayerAttack finished."));
}

bool UPlayerAttackComponent::TryCommitBufferedCombo()
{
	if (false == bAttackActive)
	{
		return false;
	}
	
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
	bCurrentAttackStaminaCostPaid = false;
	
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
	
	AnimInstance->Montage_JumpToSection(NextSectionName, CurrentAttackMontage);
	
	const FCombatAttackEntry* CurrentAttackEntry = GetCurrentComboAttackData();
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("PlayerAttack combo committed. Section: %s, Damage: %.1f"),
		*NextSectionName.ToString(), nullptr != CurrentAttackEntry ? CurrentAttackEntry->Damage : 0.0f);
	
	OnComboStateChanged.Broadcast();
	
	return true;
}

bool UPlayerAttackComponent::CanMoveToNextCombo() const
{
	if (nullptr == CurrentCombatAttackData)
	{
		return false;
	}
	
	return CurrentCombatAttackData->Attacks.IsValidIndex(CurrentComboIndex + 1);
}

const FCombatAttackEntry* UPlayerAttackComponent::GetCurrentComboAttackData() const
{
	return GetComboAttackDataByIndex(CurrentComboIndex);
}

const FCombatAttackEntry* UPlayerAttackComponent::GetComboAttackDataByIndex(int32 ComboIndex) const
{
	if (nullptr == CurrentCombatAttackData)
	{
		return nullptr;
	}
	
	if (false == CurrentCombatAttackData->Attacks.IsValidIndex(ComboIndex))
	{
		return nullptr;
	}
	
	return &CurrentCombatAttackData->Attacks[ComboIndex];
}

FName UPlayerAttackComponent::GetCurrentComboSectionName() const
{
	const FCombatAttackEntry* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
	{
		return NAME_None;
	}

	return CurrentAttackData->SectionName;
}

void UPlayerAttackComponent::ResetComboState()
{
	CurrentComboIndex = 0;
	bComboInputWindowOpen = false;
	bComboInputBuffered = false;
	
	OnComboStateChanged.Broadcast();
}

UAnimInstance* UPlayerAttackComponent::GetOwnerAnimInstance() const
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

void UPlayerAttackComponent::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentAttackMontage)
	{
		return;
	}

	FinishAttack();
}

void UPlayerAttackComponent::PerformAttackTrace()
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
	
	const FCombatAttackEntry* CurrentAttackData = GetCurrentComboAttackData();
	
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

		ApplyDamageToHitActor(HitResult);
	}
}

void UPlayerAttackComponent::ApplyDamageToHitActor(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	
	if (nullptr == HitActor)
	{
		return;
	}
	
	const FCombatAttackEntry* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
	{
		return;
	}
	
	FCombatDamageInfo DamageInfo = UCombatDamageLibrary::MakeDamageInfoFromAttackEntry(*CurrentAttackData, HitResult, GetOwner());

	const bool bDamageApplied = UCombatDamageLibrary::TryApplyCombatDamage(DamageInfo);
	
	if (false == bDamageApplied)
	{
		UE_LOG(LogCombatPortfolio, Log, TEXT("Player damage was not applied to: %s"), *GetNameSafe(HitActor));
		return;
	}
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Player applied %.1f damage to %s"), DamageInfo.DamageAmount, *GetNameSafe(HitActor));
}

bool UPlayerAttackComponent::HasAlreadyHitActor(const AActor* HitActor) const
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

void UPlayerAttackComponent::RegisterHitActor(AActor* HitActor)
{
	if (nullptr == HitActor)
	{
		return;
	}
	
	HitActorsThisAttack.Add(HitActor);
}

FVector UPlayerAttackComponent::GetAttackTraceStartLocation() const
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return FVector::ZeroVector;
	}
	
	const FCombatAttackEntry* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
	{
		return FVector::ZeroVector;
	}
	
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector ForwardVector = OwnerActor->GetActorForwardVector();
	
	return OwnerLocation + ForwardVector * CurrentAttackData->TraceForwardOffset - FVector(0.0f, 0.0f, CurrentAttackData->TraceHalfHeight);
}

FVector UPlayerAttackComponent::GetAttackTraceEndLocation() const
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return FVector::ZeroVector;
	}
	
	const FCombatAttackEntry* CurrentAttackData = GetCurrentComboAttackData();
	
	if (nullptr == CurrentAttackData)
	{
		return FVector::ZeroVector;
	}
	
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector ForwardVector = OwnerActor->GetActorForwardVector();
	
	return OwnerLocation + ForwardVector * CurrentAttackData->TraceForwardOffset + FVector(0.0f, 0.0f, CurrentAttackData->TraceHalfHeight);
}

UCombatAttackData* UPlayerAttackComponent::GetAttackDataByInputType(ECombatAttackInputType AttackInputType) const
{
	switch (AttackInputType)
	{
	case ECombatAttackInputType::Light:
		return LightAttackData;
	case ECombatAttackInputType::Heavy:
		return HeavyAttackData;
	default:
		return nullptr;
	}
}

UAnimMontage* UPlayerAttackComponent::GetAttackMontageByInputType(ECombatAttackInputType AttackInputType) const
{
	switch (AttackInputType)
	{
	case ECombatAttackInputType::Light:
		return LightAttackMontage;
	case ECombatAttackInputType::Heavy:
		return HeavyAttackMontage;
	default:
		return nullptr;
	}
}

UStaminaComponent* UPlayerAttackComponent::GetOwnerStaminaComponent() const
{
	AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return nullptr;
	}
	
	return OwnerActor->FindComponentByClass<UStaminaComponent>();
}

bool UPlayerAttackComponent::CanPayAttackStaminaCost(const FCombatAttackEntry& AttackEntry) const
{
	if (0.0f >= AttackEntry.StaminaCost)
	{
		return true;
	}
	
	const UStaminaComponent* StaminaComponent = GetOwnerStaminaComponent();
	
	if (nullptr == StaminaComponent)
	{
		return false;
	}
	
	return StaminaComponent->HasEnoughStamina(AttackEntry.StaminaCost);
}

bool UPlayerAttackComponent::TrySpendAttackStaminaCost(const FCombatAttackEntry& AttackEntry) const
{
	if (0.0f >= AttackEntry.StaminaCost)
	{
		return true;
	}
	
	UStaminaComponent* StaminaComponent = GetOwnerStaminaComponent();
	
	if (nullptr == StaminaComponent)
	{
		return false;
	}
	
	return StaminaComponent->TrySpendStamina(AttackEntry.StaminaCost);
}

void UPlayerAttackComponent::SetHitWindowOpen(bool bNewHitWindowOpen)
{
	if (bHitWindowOpen == bNewHitWindowOpen)
	{
		return;
	}
	
	bHitWindowOpen = bNewHitWindowOpen;
	
	OnHitWindowChanged.Broadcast();
}

void UPlayerAttackComponent::SetComboInputWindowOpen(bool bNewComboInputWindowOpen)
{
	if (bComboInputWindowOpen == bNewComboInputWindowOpen)
	{
		return;
	}
	
	bComboInputWindowOpen = bNewComboInputWindowOpen;
	
	OnComboStateChanged.Broadcast();
}

void UPlayerAttackComponent::SetComboInputBuffered(bool bNewComboInputBuffered)
{
	if (bComboInputBuffered == bNewComboInputBuffered)
	{
		return;
	}
	
	bComboInputBuffered = bNewComboInputBuffered;
	
	OnComboStateChanged.Broadcast();
}

