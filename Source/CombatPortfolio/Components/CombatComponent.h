#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatPortfolio/Combat/CombatDamageType.h"
#include "CombatPortfolio/Data/CombatAttackData.h"
#include "CombatComponent.generated.h"

class UPlayerAttackComponent;
class UPlayerDefenseComponent;
class UPlayerReactionComponent;

UENUM(BlueprintType)
enum class ECombatActionState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Attacking UMETA(DisplayName = "Attacking"),
	Dodging UMETA(DisplayName = "Dodging"),
	Parrying UMETA(DisplayName = "Parrying"),
	HitReaction UMETA(DisplayName = "HitReaction"),
	Dead UMETA(DisplayName = "Dead"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatActionStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitWindowChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvincibilityChangedSignature);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class COMBATPORTFOLIO_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	bool RequestAttack(ECombatAttackInputType AttackInputType);
	bool RequestCriticalAttack(AActor* CriticalTargetActor);
	bool RequestDodge(const FVector& DodgeDirection);
	bool RequestDeath();
	bool RequestParry();
	
	bool CanStartAttack() const;
	bool CanStartDodge() const;
	bool CanStartParry() const;
	
	bool IsAttacking() const;
	bool IsDodging() const;
	bool IsDead() const;
	bool IsParrying() const;
	bool IsInvincible() const;
	bool IsHitWindowOpen() const;
	bool IsComboInputWindowOpen() const;
	bool HasBufferedComboInput() const;
	
	
	int GetCurrentComboIndex() const;
	int32 GetHitActorCountThisAttack() const;
	const FCombatAttackEntry* GetCurrentAttackEntry() const;
	ECombatActionState GetCombatActionState() const;
	
	void BeginHitWindow();
	void EndHitWindow();
	
	void BeginComboInputWindow();
	void EndComboInputWindow();
	
	bool RequestHitReaction(const FCombatDamageInfo& DamageInfo);
	bool IsHitReacting() const;
	
	bool IsDeathMontageFinished() const;
	
	void BeginParryWindow();
	void EndParryWindow();
	bool TryParryIncomingDamage(const FCombatDamageInfo& DamageInfo);
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnCombatActionStateChangedSignature OnCombatActionStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnHitWindowChangedSignature OnHitWindowChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnComboStateChangedSignature OnComboStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnInvincibilityChangedSignature OnInvincibilityChanged;
	
private:
	void SetCombatActionState(ECombatActionState NewCombatActionState);
	
	void CancelCurrentActionForInterrupt();
	void CancelAttack();

	
	void CacheOwnerComponents();
	void HandlePlayerAttackFinished();
	void HandlePlayerDodgeFinished();
	void HandlePlayerParryFinished();
	void HandlePlayerDefenseInvincibilityChanged();
	void HandlePlayerHitReactionFinished();
	void HandlePlayerDeathMontageFinished();
	void HandlePlayerReactionInvincibilityChanged();
	
private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	ECombatActionState CombatActionState = ECombatActionState::Idle;

private:
	UPROPERTY()
	TObjectPtr<UPlayerAttackComponent> PlayerAttackComponent;
	
	UPROPERTY()
	TObjectPtr<UPlayerDefenseComponent> PlayerDefenseComponent;
	
	UPROPERTY()
	TObjectPtr<UPlayerReactionComponent> PlayerReactionComponent;
};
