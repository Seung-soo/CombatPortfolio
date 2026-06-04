#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatPortfolio/Combat/CombatDamageType.h"
#include "PlayerReactionComponent.generated.h"

class UAnimMontage;
class UAnimInstance;

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UPlayerReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerReactionComponent();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	bool StartHitReaction(const FCombatDamageInfo& DamageInfo);
	void CancelHitReaction();
	bool IsHitReacting() const;
	bool IsHitReactionInvincible() const;
	
	bool StartDeath();
	bool IsDeathMontageFinished() const;
	
public:
	FSimpleMulticastDelegate OnHitReactionFinished;
	FSimpleMulticastDelegate OnDeathMontageFinished;
	FSimpleMulticastDelegate OnInvincibilityChanged;
	
private:
	bool TryPlayHitReactionMontage(const FCombatDamageInfo& DamageInfo);
	UAnimMontage* GetHitReactionMontageByDirection(ECombatHitDirection HitDirection) const;
	void FinishHitReaction();
	void HandleHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void StartHitReactionInvincibility();
	void EndHitReactionInvincibility();
	void SetHitReactionInvincible(bool bNewInvincible);
	
	bool TryPlayDeathMontage();
	void HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	UAnimInstance* GetOwnerAnimInstance() const;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction|Hit", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float HitReactionInvincibleDuration = 0.35f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction|Hit", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> FrontHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction|Hit", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> BackHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction|Hit", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> LeftHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction|Hit", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> RightHitReactionMontage;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Reaction|Hit", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> CurrentHitReactionMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction|Hit", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float HitReactionMontagePlayRate = 1.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Reaction|Hit", meta = (AllowPrivateAccess = "true"))
	bool bHitReacting = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Reaction|Hit", meta = (AllowPrivateAccess = "true"))
	bool bHitReactionInvincible = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction|Death", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DeathMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction|Death", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float DeathMontagePlayRate = 1.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Reaction|Death", meta = (AllowPrivateAccess = "true"))
	bool bDeathMontageFinished = false;
	
private:
	FTimerHandle HitReactionInvincibleTimerHandle;
};
