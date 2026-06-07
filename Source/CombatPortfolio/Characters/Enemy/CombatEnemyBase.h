#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "CombatEnemyBase.generated.h"

class UStaticMeshComponent;
class UHealthComponent;
class UEnemyAttackComponent;
class ULockOnMarkerComponent;
class UEnemyHealthBarComponent;
class UHitStopComponent;

UCLASS()
class COMBATPORTFOLIO_API ACombatEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	ACombatEnemyBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:
	UFUNCTION()
	virtual void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta);
	
	UFUNCTION()
	virtual void HandleDeath();
	
	virtual void ApplyDeathState();
	

	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> BodyMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UEnemyAttackComponent> EnemyAttackComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<ULockOnMarkerComponent> LockOnMarkerComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UEnemyHealthBarComponent> EnemyHealthBarComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UHitStopComponent> HitStopComponent;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Critical", meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
	float CriticalAttackWindowDuration = 2.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Critical", meta = (AllowPrivateAccess = true))
	bool bCriticalAttackAvailable = false;
	
	FTimerHandle CriticalAttackWindowTimerHandle;

public:
	virtual bool RequestParriedReaction();
	
	virtual void OpenCriticalAttackWindow();

public:
	UHealthComponent* GetHealthComponent() const;
	UEnemyAttackComponent* GetEnemyAttackComponent() const;
	ULockOnMarkerComponent* GetLockOnMarkerComponent() const;
	UEnemyHealthBarComponent* GetEnemyHealthBarComponent() const;
	
public:
	bool ConsumeCriticalAttackOpportunity(AActor* AttackerActor);
	bool IsCriticalAttackAvailable() const;
	
private:
	void CloseCriticalAttackWindow();
};
