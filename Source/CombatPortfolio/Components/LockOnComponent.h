// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockOnTargetChangedSignature);

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULockOnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	bool ToggleLockOn();
	bool TryLockOn();
	void ClearLockOnTarget();
	bool IsLockedOn() const;
	AActor* GetLockOnTarget() const;

public:
	UPROPERTY(BlueprintAssignable, Category = "LockOn|Event")
	FOnLockOnTargetChangedSignature OnLockOnTargetChaged;
	
private:
	AActor* FindBestLockOnTarget() const;
	bool IsValidLockOnTarget(const AActor* CandidateActor) const;
	bool IsTargetInsideLockOnAngle(const AActor* CandidateActor) const;
	float GetDistanceToTarget(const AActor* CandidateActor) const;
	void SetLockOnTarget(AActor* NewTarget);
	void UpdateLockOnValidation();
	
	void ShowTargetMarker(AActor* TargetActor) const;
	void HideTargetMarker(AActor* TargetActor) const;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (AllowPrivateAccess = "true", ClampMin = "100.0"))
	float LockOnRadius = 1200.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "180.0"))
	float LockOnAngle = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (AllowPrivateAccess = "true"))
	bool bDrawLockOnDebug = true;
	
private:
	TWeakObjectPtr<AActor> LockOnTarget;
};
