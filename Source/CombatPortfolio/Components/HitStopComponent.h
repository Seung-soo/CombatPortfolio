// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitStopComponent.generated.h"


UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class COMBATPORTFOLIO_API UHitStopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHitStopComponent();
	
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	void RequestHitStop(float Duration, float TimeDilation = 0.05f);

private:
	void EndHitStop();
		
private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Hit Stop", meta = (AllowPrivateAccess = "true"))
	bool bHitStopActive = false;
	
private:
	float OriginalCustomTimeDilation = 1.0f;
	FTimerHandle HitStopTimerHandle;
};
