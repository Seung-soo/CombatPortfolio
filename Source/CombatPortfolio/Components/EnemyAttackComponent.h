#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "EnemyAttackComponent.generated.h"

struct FCombatAttackEntry;
class UCombatAttackData;
class UAnimMontage;
class UCombatComponent;
class UHealthComponent;

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UEnemyAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEnemyAttackComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	void StartAutoAttack();
	void StopAutoAttack();
	void RequestAutoAttack();
	
	bool RequestAttack();
	bool CanRequestAttack() const;
	bool IsAttackOnCooldown() const;
	bool IsAttacking() const;
	bool IsHitWindowOpen() const;
	void SetStartAttackOnBeginPlay(bool bNewStartAttackOnBeginPlay);
	
	void OpenAttackHitWindow();
	void CloseAttackHitWindow();
	void EndAttack();
	void CancelAttack();
	
private:
	bool TryPlayAttackMontage();
	void PerformAttackTrace();
	void ApplyDamageToActor(const FHitResult& HitResult);
	
	bool IsDamageBlockedByInvincibility(AActor* TargetActor) const;
	
	bool HasAlreadyHitActor(const AActor* TargetActor) const;
	
	void AddHitActor(AActor* TargetActor);
	void ResetHitActors();
	
	FVector GetAttackTraceStartLocation() const;
	FVector GetAttackTraceEndLocation() const;
	
	void BeginAttackCooldown();
	void EndAttackCooldown();
	
	void SetComponentTickByHitWindow();
	
	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	const FCombatAttackEntry* GetAttackEntry() const;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	bool bStartAttackOnBeginPlay = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float AttackInterval = 2.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AttackCooldown = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Animation", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float AttackMontagePlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawAttackDebug = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	bool bAttackOnCooldown = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	bool bAttacking = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	bool bHitWindowOpen = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatAttackData> AttackData;
	
private:
	TArray<TWeakObjectPtr<AActor>> HitActorsThisAttack;
	
	FTimerHandle AutoAttackTimerHandle;
	FTimerHandle AttackCooldownTimerHandle;
};
