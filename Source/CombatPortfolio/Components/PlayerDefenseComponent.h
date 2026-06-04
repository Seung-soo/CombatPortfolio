#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatPortfolio/Combat/CombatDamageType.h"
#include "PlayerDefenseComponent.generated.h"

class UAnimMontage;
class UAnimInstance;
class UStaminaComponent;
class UCameraShakeBase;

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UPlayerDefenseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerDefenseComponent();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	bool StartDodge(const FVector& DodgeDirection);
	bool CanStartDodge() const;
	void CancelDodge();
	bool IsDodging() const;
	bool IsDodgeInvincible() const;
	
	bool StartParry();
	void CancelParry();
	bool IsParrying() const;
	bool IsParryWindowOpen() const;
	
	void BeginParryWindow();
	void EndParryWindow();
	bool TryParryIncomingDamage(const FCombatDamageInfo& DamageInfo);
	
public:
	FSimpleMulticastDelegate OnDodgeFinished;
	FSimpleMulticastDelegate OnParryFinished;
	FSimpleMulticastDelegate OnInvincibilityChanged;
	
private:
	bool TryPlayDodgeMontage();
	void ApplyDodgeMovement(const FVector& DodgeDirection);
	void BeginDodgeInvincibility();
	void EndDodgeInvincibility();
	void FinishDodge();
	void HandleDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	bool TryPlayParryMontage();
	bool TryPlayParrySuccessMontage();
	void FinishParry();
	void HandleParryMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void HandleParrySuccessMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void SetDodgeInvincible(bool bNewInvincible);
	void SetParryWindowOpen(bool bNewParryWindowOpen);
	void ApplyParrySuccessFeedback(const FCombatDamageInfo& DamageInfo);
	
	UAnimInstance* GetOwnerAnimInstance() const;
	UStaminaComponent* GetOwnerStaminaComponent() const;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Dodge", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DodgeMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Dodge", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float DodgePlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Dodge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DodgeStrength = 900.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Dodge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0" ))
	float DodgeDuration = 0.45f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Dodge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DodgeInvincibleDuration = 0.25f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Dodge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DodgeStaminaCost = 25.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Defense|Dodge", meta = (AllowPrivateAccess = "true"))
	bool bDodging = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Defense|Dodge", meta = (AllowPrivateAccess = "true"))
	bool bDodgeInvincible = false;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ParryMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ParrySuccessMontage;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> CurrentParryMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float ParryMontagePlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float ParrySuccessMontagePlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ParryStaminaCost = 15.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true"))
	bool bParrying = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true"))
	bool bParryWindowOpen = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true"))
	bool bParrySucceeded = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry|Feedback", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ParrySuccessHitStopDuration = 0.08f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ClampMax = "1.0"))
	float ParrySuccessHitStopTimeDilation = 0.03f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry|Feedback", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> ParrySuccessCameraShakeClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry|Feedback", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ParrySuccessCameraShakeScale = 1.0f;
	
private:
	FTimerHandle DodgeFallbackTimerHandle;
	FTimerHandle DodgeInvincibilityTimerHandle;
};
