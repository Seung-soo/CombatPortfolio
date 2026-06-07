#include "CombatComponent.h"
#include "PlayerAttackComponent.h"
#include "PlayerDefenseComponent.h"
#include "PlayerReactionComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CacheOwnerComponents();
	
	SetComponentTickEnabled(false);
}

void UCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

bool UCombatComponent::RequestAttack(ECombatAttackInputType AttackInputType)
{
	if (nullptr == PlayerAttackComponent)
	{
		return false;
	}
	
	if (ECombatActionState::Idle == CombatActionState)
	{
		const bool bAttackStarted = PlayerAttackComponent->StartAttack(AttackInputType);
		
		if (false == bAttackStarted)
		{
			return false;
		}
		
		SetCombatActionState(ECombatActionState::Attacking);
		return true;
	}
	
	if (ECombatActionState::Attacking == CombatActionState)
	{
		if (ECombatAttackInputType::Light == AttackInputType)
		{
			return PlayerAttackComponent->TryBufferComboInput();
		}
		
		return false;
	}
	
	return false;
}

bool UCombatComponent::RequestCriticalAttack(AActor* CriticalTargetActor)
{
	if (nullptr == PlayerAttackComponent)
	{
		return false;
	}
	
	if (ECombatActionState::Idle != CombatActionState)
	{
		return false;
	}
	
	const bool bAttackStarted = PlayerAttackComponent->StartCriticalAttack(CriticalTargetActor);
	
	if (false == bAttackStarted)
	{
		return false;
	}
	
	SetCombatActionState(ECombatActionState::Attacking);
	
	return true;
}

bool UCombatComponent::RequestDodge(const FVector& DodgeDirection)
{
	if (nullptr == PlayerDefenseComponent)
	{
		return false;
	}
	
	if (false == CanStartDodge())
	{
		return false;
	}
	
	const bool bDodgeStarted = PlayerDefenseComponent->StartDodge(DodgeDirection);
	
	if (false == bDodgeStarted)
	{
		return false;
	}
	
	SetCombatActionState(ECombatActionState::Dodging);
	
	return true;
}

bool UCombatComponent::RequestDeath()
{
	if (ECombatActionState::Dead == CombatActionState)
	{
		return false;
	}
	
	CancelCurrentActionForInterrupt();
	
	SetCombatActionState(ECombatActionState::Dead);

	if (nullptr != PlayerReactionComponent)
	{
		PlayerReactionComponent->StartDeath();
	}
	
	return true;
}

bool UCombatComponent::RequestParry()
{
	if (nullptr == PlayerDefenseComponent)
	{
		return false;
	}
	
	if (false == CanStartParry())
	{
		return false;
	}
	
	const bool bParryStarted = PlayerDefenseComponent->StartParry();
	
	if (false == bParryStarted)
	{
		return false;
	}
	
	SetCombatActionState(ECombatActionState::Parrying);
	
	return true;
}

bool UCombatComponent::CanStartDodge() const
{
	if (ECombatActionState::Idle != CombatActionState)
	{
		return false;
	}
	
	if (nullptr == PlayerDefenseComponent)
	{
		return false;
	}
	
	return PlayerDefenseComponent->CanStartDodge();
}

bool UCombatComponent::CanStartParry() const
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

bool UCombatComponent::IsParrying() const
{
	return ECombatActionState::Parrying == CombatActionState;
}

bool UCombatComponent::IsInvincible() const
{
	const bool bDefenseInvincible = nullptr != PlayerDefenseComponent && 
		true == PlayerDefenseComponent->IsDodgeInvincible();
	
	const bool bReactionInvincible = nullptr != PlayerReactionComponent && 
		true == PlayerReactionComponent->IsHitReactionInvincible();
	
	return bDefenseInvincible || bReactionInvincible;
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
	return nullptr != PlayerAttackComponent && PlayerAttackComponent->IsHitWindowOpen();
}

bool UCombatComponent::IsComboInputWindowOpen() const
{
	return nullptr != PlayerAttackComponent && PlayerAttackComponent->IsComboInputWindowOpen();
}

bool UCombatComponent::HasBufferedComboInput() const
{
	return nullptr != PlayerAttackComponent && PlayerAttackComponent->HasBufferedComboInput();
}

int UCombatComponent::GetCurrentComboIndex() const
{
	return nullptr != PlayerAttackComponent ? PlayerAttackComponent->GetCurrentComboIndex() : 0;
}

int32 UCombatComponent::GetHitActorCountThisAttack() const
{
	return nullptr != PlayerAttackComponent ? PlayerAttackComponent->GetHitActorCountThisAttack() : 0;
}

const FCombatAttackEntry* UCombatComponent::GetCurrentAttackEntry() const
{
	return nullptr != PlayerAttackComponent ? PlayerAttackComponent->GetCurrentAttackEntry() : nullptr;
}

void UCombatComponent::BeginHitWindow()
{
	if (nullptr != PlayerAttackComponent)
	{
		PlayerAttackComponent->BeginHitWindow();
	}
}

void UCombatComponent::EndHitWindow()
{
	if (nullptr != PlayerAttackComponent)
	{
		PlayerAttackComponent->EndHitWindow();
	}
}

void UCombatComponent::BeginComboInputWindow()
{
	if (nullptr != PlayerAttackComponent)
	{
		PlayerAttackComponent->BeginComboInputWindow();
	}
}

void UCombatComponent::EndComboInputWindow()
{
	if (nullptr != PlayerAttackComponent)
	{
		PlayerAttackComponent->EndComboInputWindow();
	}
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
	
	if (nullptr == PlayerReactionComponent)
	{
		return false;
	}
	
	CancelCurrentActionForInterrupt();
	
	SetCombatActionState(ECombatActionState::HitReaction);
	
	const bool bHitReactionStarted = PlayerReactionComponent->StartHitReaction(DamageInfo);
	
	if (false == bHitReactionStarted)
	{
		SetCombatActionState(ECombatActionState::Idle);
		return false;
	}

	return true;
}

bool UCombatComponent::IsHitReacting() const
{
	return ECombatActionState::HitReaction == CombatActionState;
}

bool UCombatComponent::IsDeathMontageFinished() const
{
	return nullptr != PlayerReactionComponent && true == PlayerReactionComponent->IsDeathMontageFinished();
}

void UCombatComponent::BeginParryWindow()
{
	if (nullptr != PlayerDefenseComponent)
	{
		PlayerDefenseComponent->BeginParryWindow();
	}
}

void UCombatComponent::EndParryWindow()
{
	if (nullptr != PlayerDefenseComponent)
	{
		PlayerDefenseComponent->EndParryWindow();
	}
}

bool UCombatComponent::TryParryIncomingDamage(const FCombatDamageInfo& DamageInfo)
{
	return nullptr != PlayerDefenseComponent && true == PlayerDefenseComponent->TryParryIncomingDamage(DamageInfo);
}

ECombatActionState UCombatComponent::GetCombatActionState() const
{
	return CombatActionState;
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

void UCombatComponent::CancelCurrentActionForInterrupt()
{
	switch (CombatActionState)
	{
	case ECombatActionState::Attacking:
		CancelAttack();
		break;
	case ECombatActionState::Dodging:
		if (nullptr != PlayerDefenseComponent)
		{
			PlayerDefenseComponent->CancelDodge();
		}
		break;
	case ECombatActionState::Parrying:
		if (nullptr != PlayerDefenseComponent)
		{
			PlayerDefenseComponent->CancelParry();
		}
		break;
	case ECombatActionState::HitReaction:
		if (nullptr != PlayerReactionComponent)
		{
			PlayerReactionComponent->CancelHitReaction();
		}
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
	if (nullptr != PlayerAttackComponent)
	{
		PlayerAttackComponent->CancelAttack();
	}
	
	if (ECombatActionState::Attacking == CombatActionState)
	{
		SetCombatActionState(ECombatActionState::Idle);
	}
}

void UCombatComponent::CacheOwnerComponents()
{
	AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	PlayerAttackComponent = OwnerActor->FindComponentByClass<UPlayerAttackComponent>();
	PlayerDefenseComponent = OwnerActor->FindComponentByClass<UPlayerDefenseComponent>();
	PlayerReactionComponent = OwnerActor->FindComponentByClass<UPlayerReactionComponent>();

	if (nullptr != PlayerAttackComponent)
	{
		PlayerAttackComponent->OnAttackFinished.AddUObject(this, &UCombatComponent::HandlePlayerAttackFinished);
		PlayerAttackComponent->OnHitWindowChanged.AddLambda([this]()
		{
			OnHitWindowChanged.Broadcast();
		});
		PlayerAttackComponent->OnComboStateChanged.AddLambda([this]()
		{
			OnComboStateChanged.Broadcast();
		});
	}
	
	if (nullptr != PlayerDefenseComponent)
	{
		PlayerDefenseComponent->OnDodgeFinished.AddUObject(this, &UCombatComponent::HandlePlayerDodgeFinished);
		PlayerDefenseComponent->OnParryFinished.AddUObject(this, &UCombatComponent::HandlePlayerParryFinished);
		PlayerDefenseComponent->OnInvincibilityChanged.AddUObject(this, &UCombatComponent::HandlePlayerDefenseInvincibilityChanged);
	}
	
	if (nullptr != PlayerReactionComponent)
	{
		PlayerReactionComponent->OnHitReactionFinished.AddUObject(this, &UCombatComponent::HandlePlayerHitReactionFinished);
		PlayerReactionComponent->OnDeathMontageFinished.AddUObject(this, &UCombatComponent::HandlePlayerDeathMontageFinished);
		PlayerReactionComponent->OnInvincibilityChanged.AddUObject(this, &UCombatComponent::HandlePlayerReactionInvincibilityChanged);
	}
}

void UCombatComponent::HandlePlayerAttackFinished()
{
	if (ECombatActionState::Attacking != CombatActionState)
	{
		return;
	}
	
	SetCombatActionState(ECombatActionState::Idle);
}

void UCombatComponent::HandlePlayerDodgeFinished()
{
	if (ECombatActionState::Dodging != CombatActionState)
	{
		return;
	}
	
	SetCombatActionState(ECombatActionState::Idle);
}

void UCombatComponent::HandlePlayerParryFinished()
{
	if (ECombatActionState::Parrying != CombatActionState)
	{
		return;
	}
	
	SetCombatActionState(ECombatActionState::Idle);
}

void UCombatComponent::HandlePlayerDefenseInvincibilityChanged()
{
	OnInvincibilityChanged.Broadcast();
}

void UCombatComponent::HandlePlayerHitReactionFinished()
{
	if (ECombatActionState::HitReaction != CombatActionState)
	{
		return;
	}
	
	SetCombatActionState(ECombatActionState::Idle);
}

void UCombatComponent::HandlePlayerDeathMontageFinished()
{
}

void UCombatComponent::HandlePlayerReactionInvincibilityChanged()
{
	OnInvincibilityChanged.Broadcast();
}
