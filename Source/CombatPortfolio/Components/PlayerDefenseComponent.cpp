#include "PlayerDefenseComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "CombatPortfolio/CombatPortfolio.h"
#include "CombatPortfolio/Combat/CombatDamageLibrary.h"
#include "CombatPortfolio/Components/StaminaComponent.h"

UPlayerDefenseComponent::UPlayerDefenseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerDefenseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	
	if (nullptr != World)
	{
		World->GetTimerManager().ClearTimer(DodgeFallbackTimerHandle);
		World->GetTimerManager().ClearTimer(DodgeInvincibilityTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

bool UPlayerDefenseComponent::StartDodge(const FVector& DodgeDirection)
{
	if (false == CanStartDodge())
	{
		return false;
	}
	
	UStaminaComponent* StaminaComponent = GetOwnerStaminaComponent();
	
	if (nullptr == StaminaComponent)
	{
		return false;
	}
	
	if (false == StaminaComponent->TrySpendStamina(DodgeStaminaCost))
	{
		UE_LOG(LogCombatPortfolio, Log, TEXT("Dodge failed: Failed to spend stamina. Required: %.1f"), DodgeStaminaCost);
		return false;
	}
	
	bDodging = true;
	
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
			&UPlayerDefenseComponent::FinishDodge,
			DodgeDuration,
			false
		);
	}
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Dodge started. MontagePlayed: %s | StaminaCost: %.1f"), 
		true == bDodgeMontagePlayed ? TEXT("true") : TEXT("false"), DodgeStaminaCost);
	
	return true;
}

bool UPlayerDefenseComponent::CanStartDodge() const
{
	if (true == bDodging)
	{
		return false;
	}
	
	UStaminaComponent* StaminaComponent = GetOwnerStaminaComponent();
	
	if (nullptr == StaminaComponent)
	{
		return false;
	}
	
	if (false == StaminaComponent->HasEnoughStamina(DodgeStaminaCost))
	{
		UE_LOG(LogCombatPortfolio, Log, TEXT("Dodge failed: Not enough stamina. Required: %.1f"), DodgeStaminaCost);
		return false;
	}
	
	return true;
}

void UPlayerDefenseComponent::CancelDodge()
{
	if (false == bDodging)
	{
		return;
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr != OwnerCharacter && nullptr != DodgeMontage)
	{
		OwnerCharacter->StopAnimMontage(DodgeMontage);
	}
	
	FinishDodge();
}

bool UPlayerDefenseComponent::IsDodging() const
{
	return bDodging;
}

bool UPlayerDefenseComponent::IsDodgeInvincible() const
{
	return bDodgeInvincible;
}

bool UPlayerDefenseComponent::StartParry()
{
	if (true == bParrying)
	{
		return false;
	}
	
	UStaminaComponent* StaminaComponent = GetOwnerStaminaComponent();
	
	if (nullptr == StaminaComponent)
	{
		return false;
	}
	
	if (false == StaminaComponent->HasEnoughStamina(ParryStaminaCost))
	{
		UE_LOG(LogCombatPortfolio, Log, TEXT("Parry failed: Not enough stamina. Required: %.1f"), ParryStaminaCost);
		return false;
	}
	
	bParrying = true;
	bParrySucceeded = false;
	CurrentParryMontage = ParryMontage;
	SetParryWindowOpen(false);
	
	if (false == TryPlayParryMontage())
	{
		bParrying = false;
		CurrentParryMontage = nullptr;
		SetParryWindowOpen(false);
		return false;
	}
	
	if (false == StaminaComponent->TrySpendStamina(ParryStaminaCost))
	{
		CancelParry();
		return false;
	}
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Parry started. StaminaCost: %.1f"), ParryStaminaCost);
	
	return true;
}

void UPlayerDefenseComponent::CancelParry()
{
	if (false == bParrying)
	{
		return;
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr != OwnerCharacter && nullptr != CurrentParryMontage)
	{
		OwnerCharacter->StopAnimMontage(CurrentParryMontage);
	}
	
	FinishParry();
}

bool UPlayerDefenseComponent::IsParrying() const
{
	return bParrying;
}

bool UPlayerDefenseComponent::IsParryWindowOpen() const
{
	return bParryWindowOpen;
}

void UPlayerDefenseComponent::BeginParryWindow()
{
	if (false == bParrying)
	{
		return;
	}
	
	SetParryWindowOpen(true);
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Parry window Opened."));
}

void UPlayerDefenseComponent::EndParryWindow()
{
	SetParryWindowOpen(false);
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Parry window closed."));
}

bool UPlayerDefenseComponent::TryParryIncomingDamage(const FCombatDamageInfo& DamageInfo)
{
	if (false == bParrying)
	{
		return false;
	}
	
	if (true == bParrySucceeded)
	{
		return false;
	}
	
	if (false == bParryWindowOpen)
	{
		return false;
	}
	
	if (ECombatHitStrength::Heavy == DamageInfo.HitStrength)
	{
		UE_LOG(LogCombatPortfolio, Log, TEXT("Parry failed: Heavy attack cannot be parried."));
		return false;
	}
	
	bParrySucceeded = true;
	
	SetParryWindowOpen(false);
	
	ApplyParrySuccessFeedback(DamageInfo);
	
	const bool bSuccessMontageStarted = TryPlayParrySuccessMontage();
	
	if (false == bSuccessMontageStarted)
	{
		FinishParry();
	}
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Parry success. Attacker: %s"),
		IsValid(DamageInfo.InstigatorActor) ? *DamageInfo.InstigatorActor->GetName() : TEXT("None"));

	return true;
}

bool UPlayerDefenseComponent::TryPlayDodgeMontage()
{
	if (nullptr == DodgeMontage)
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("DodgeMontage is not assigned. Dodge movement will still happen"));
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
		UE_LOG(LogCombatPortfolio, Warning, TEXT("Dodge Montage_Play failed."));
		return false;
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerDefenseComponent::HandleDodgeMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DodgeMontage);
	
	return true;
}

void UPlayerDefenseComponent::ApplyDodgeMovement(const FVector& DodgeDirection)
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

void UPlayerDefenseComponent::BeginDodgeInvincibility()
{
	SetDodgeInvincible(true);
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(DodgeInvincibilityTimerHandle);
	
	World->GetTimerManager().SetTimer(
		DodgeInvincibilityTimerHandle,
		this,
		&UPlayerDefenseComponent::EndDodgeInvincibility,
		DodgeInvincibleDuration,
		false
	);
}

void UPlayerDefenseComponent::EndDodgeInvincibility()
{
	SetDodgeInvincible(false);
}

void UPlayerDefenseComponent::FinishDodge()
{
	if (false == bDodging)
	{
		return;
	}
	
	UWorld* World = GetWorld();
	
	if (nullptr != World)
	{
		World->GetTimerManager().ClearTimer(DodgeFallbackTimerHandle);
	}
	
	EndDodgeInvincibility();
	
	bDodging = false;
	
	OnDodgeFinished.Broadcast();
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Dodge finished."));
}

void UPlayerDefenseComponent::HandleDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != DodgeMontage)
	{
		return;
	}
	
	FinishDodge();
}

bool UPlayerDefenseComponent::TryPlayParryMontage()
{
	if (nullptr == ParryMontage)
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("Parry failed: ParryMontage is null."));
		return false;
	}
	
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		return false;
	}
	
	const float MontageDuration = AnimInstance->Montage_Play(ParryMontage, ParryMontagePlayRate);
	
	if (0.0f >= MontageDuration)
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("Parry failed: Montage_Play returned 0."));
		return false;
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerDefenseComponent::HandleParryMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ParryMontage);
	
	return true;
}

bool UPlayerDefenseComponent::TryPlayParrySuccessMontage()
{
	if (nullptr == ParrySuccessMontage)
	{
		return false;
	}
	
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		return false;
	}
	
	CurrentParryMontage = ParrySuccessMontage;
	
	const float MontageDuration = AnimInstance->Montage_Play(ParrySuccessMontage, ParrySuccessMontagePlayRate);
	
	if (0.0f >= MontageDuration)
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("ParrySuccess failed: Montage_Play returned 0."));
		FinishParry();
		return false;
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerDefenseComponent::HandleParrySuccessMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ParrySuccessMontage);
	
	return true;
}

void UPlayerDefenseComponent::FinishParry()
{
	if (false == bParrying)
	{
		return;
	}
	
	SetParryWindowOpen(false);
	
	CurrentParryMontage = nullptr;
	bParrySucceeded = false;
	bParrying = false;
	
	OnParryFinished.Broadcast();
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Parry finished."));
}

void UPlayerDefenseComponent::HandleParryMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentParryMontage)
	{
		return;
	}
	
	if (true == bParrySucceeded)
	{
		return;
	}
	
	FinishParry();
}

void UPlayerDefenseComponent::HandleParrySuccessMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentParryMontage)
	{
		return;
	}
	
	FinishParry();
}

void UPlayerDefenseComponent::SetDodgeInvincible(bool bNewInvincible)
{
	if (bDodgeInvincible == bNewInvincible)
	{
		return;
	}
	
	bDodgeInvincible = bNewInvincible;
	OnInvincibilityChanged.Broadcast();
}

void UPlayerDefenseComponent::SetParryWindowOpen(bool bNewParryWindowOpen)
{
	if (bParryWindowOpen == bNewParryWindowOpen)
	{
		return;
	}
	
	bParryWindowOpen = bNewParryWindowOpen;
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Parry window %s."), true == bParryWindowOpen ? TEXT("opened") : TEXT("closed"));
}

void UPlayerDefenseComponent::ApplyParrySuccessFeedback(const FCombatDamageInfo& DamageInfo)
{
	FCombatDamageInfo ParryFeedbackInfo = DamageInfo;
	
	ParryFeedbackInfo.DamageAmount = 0.0f;
	ParryFeedbackInfo.HitStopDuration = ParrySuccessHitStopDuration;
	ParryFeedbackInfo.HitStopTimeDilation = ParrySuccessHitStopTimeDilation;
	ParryFeedbackInfo.CameraShakeClass = ParrySuccessCameraShakeClass;
	ParryFeedbackInfo.CameraShakeScale = ParrySuccessCameraShakeScale;
	ParryFeedbackInfo.HitVFX = nullptr;
	ParryFeedbackInfo.HitSFX = nullptr;
	
	UCombatDamageLibrary::ApplyDamageFeedbackFromDamageInfo(ParryFeedbackInfo);
}

UAnimInstance* UPlayerDefenseComponent::GetOwnerAnimInstance() const
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

UStaminaComponent* UPlayerDefenseComponent::GetOwnerStaminaComponent() const
{
	AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return nullptr;
	}
	
	return OwnerActor->FindComponentByClass<UStaminaComponent>();
}
