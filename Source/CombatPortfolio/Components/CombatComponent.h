// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "CombatComponent.generated.h"

class UAnimInstance;
class UAnimMontage;

UENUM(BlueprintType)
enum class ECombatActionState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Attacking UMETA(DisplayName = "Attacking"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatActionStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitWindowChangedSignature);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class COMBATPORTFOLIO_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	bool RequestAttack();
	bool CanStartAttack() const;
	bool IsAttacking() const;
	bool IsHitWindowOpen() const;
	ECombatActionState GetCombatActionState() const;
	
	void BeginHitWindow();
	void EndHitWindow();
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnCombatActionStateChangedSignature OnCombatActionStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnHitWindowChangedSignature OnHitWindowChanged;
	
private:
	bool StartAttack();
	void FinishAttack();
	
	UAnimInstance* GetOwnerAnimInstance() const;
	
	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void SetCombatActionState(ECombatActionState NewCombatActionState);
	
	void SetHitWindowOpen(bool bNewHitWindowOpen);
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	float AttackPlayRate = 1.0f;
		
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	ECombatActionState CombatActionState = ECombatActionState::Idle;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
	bool bHitWindowOpen = false;
};
