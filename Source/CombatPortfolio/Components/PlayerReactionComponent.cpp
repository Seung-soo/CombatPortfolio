#include "PlayerReactionComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "CombatPortfolio/CombatPortfolio.h"

UPlayerReactionComponent::UPlayerReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	
	if (nullptr != World)
	{
		World->GetTimerManager().ClearTimer(HitReactionInvincibleTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

bool UPlayerReactionComponent::StartHitReaction(const FCombatDamageInfo& DamageInfo)
{
	if (true == bHitReacting)
	{
		return false;
	}
	
	bHitReacting = true;
	
	StartHitReactionInvincibility();
	
	const bool bPlayedMontage = TryPlayHitReactionMontage(DamageInfo);
	
	if (false == bPlayedMontage)
	{
		UE_LOG(LogCombatPortfolio, Error, TEXT("HitReactionMontage failed. Strength: %s, Direction: %s"),
			GetCombatHitStrengthDebugString(DamageInfo.HitStrength),
			GetCombatHitDirectionDebugString(DamageInfo.HitDirectionType));
		
		FinishHitReaction();
		return false;
	}
	
	return true;
}

void UPlayerReactionComponent::CancelHitReaction()
{
	if (false == bHitReacting)
	{
		return;
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr != OwnerCharacter && nullptr != CurrentHitReactionMontage)
	{
		OwnerCharacter->StopAnimMontage(CurrentHitReactionMontage);
	}
	
	FinishHitReaction();
}

bool UPlayerReactionComponent::IsHitReacting() const
{
	return bHitReacting;
}

bool UPlayerReactionComponent::IsHitReactionInvincible() const
{
	return bHitReactionInvincible;
}

bool UPlayerReactionComponent::StartDeath()
{
	bDeathMontageFinished = false;
	
	CancelHitReaction();
	
	const bool bDeathMontageStarted = TryPlayDeathMontage();
	
	if (false == bDeathMontageStarted)
	{
		UE_LOG(LogCombatPortfolio, Error, TEXT("DeathMontage is not assigned or failed to play."));
	}
	
	return true;
}

bool UPlayerReactionComponent::IsDeathMontageFinished() const
{
	return bDeathMontageFinished;
}

bool UPlayerReactionComponent::TryPlayHitReactionMontage(const FCombatDamageInfo& DamageInfo)
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
	
	const float MontageDuration = AnimInstance->Montage_Play(HitReactionMontage, HitReactionMontagePlayRate);
	
	if (0.0f >= MontageDuration)
	{
		CurrentHitReactionMontage = nullptr;
		return false;
	}
	
	CurrentHitReactionMontage = HitReactionMontage;
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerReactionComponent::HandleHitReactionMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, HitReactionMontage);
	
	return true;
}

UAnimMontage* UPlayerReactionComponent::GetHitReactionMontageByDirection(ECombatHitDirection HitDirection) const
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

void UPlayerReactionComponent::FinishHitReaction()
{
	if (false == bHitReacting)
	{
		return;
	}
	
	CurrentHitReactionMontage = nullptr;
	bHitReacting = false;
	
	OnHitReactionFinished.Broadcast();
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Player hit reaction ended."));
}

void UPlayerReactionComponent::HandleHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentHitReactionMontage)
	{
		return;
	}

	FinishHitReaction();
}

void UPlayerReactionComponent::StartHitReactionInvincibility()
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
		&UPlayerReactionComponent::EndHitReactionInvincibility,
		HitReactionInvincibleDuration,
		false
	);
}

void UPlayerReactionComponent::EndHitReactionInvincibility()
{
	SetHitReactionInvincible(false);
}

void UPlayerReactionComponent::SetHitReactionInvincible(bool bNewInvincible)
{
	if (bHitReactionInvincible == bNewInvincible)
	{
		return;
	}
	
	bHitReactionInvincible = bNewInvincible;
	
	OnInvincibilityChanged.Broadcast();
}

bool UPlayerReactionComponent::TryPlayDeathMontage()
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

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		return false;
	}
	
	const float MontageLength = OwnerCharacter->PlayAnimMontage(DeathMontage, DeathMontagePlayRate);
	
	if (0.0f >= MontageLength)
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("%s failed to play DeathMontage."), *OwnerCharacter->GetName());
		return false;
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UPlayerReactionComponent::HandleDeathMontageEnded);
	
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DeathMontage);
	
	return true;
}

void UPlayerReactionComponent::HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != DeathMontage)
	{
		return;
	}
	
	bDeathMontageFinished = true;
	
	OnDeathMontageFinished.Broadcast();
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("Player death montage ended. Interrupted: %s"), bInterrupted ? TEXT("true") : TEXT("false"));
}

UAnimInstance* UPlayerReactionComponent::GetOwnerAnimInstance() const
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
