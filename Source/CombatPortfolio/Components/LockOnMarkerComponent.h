// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "LockOnMarkerComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class COMBATPORTFOLIO_API ULockOnMarkerComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	ULockOnMarkerComponent();
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	void ShowMarker();
	void HideMarker();
	bool IsMarkerVisible() const;
	
private:
	void UpdateMarkerWorldLocation();
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn Marker", meta = (AllowPrivateAccess = "true"))
	float MarkerHeight = 130.0f;
};
