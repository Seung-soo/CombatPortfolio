// Fill out your copyright notice in the Description page of Project Settings.


#include "LockOnMarkerComponent.h"

ULockOnMarkerComponent::ULockOnMarkerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawSize(FVector2D(64.0f, 64.0f));
	SetPivot(FVector2D(0.5f, 0.5f));
	
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	
	SetVisibility(false, true);
	SetHiddenInGame(true);
}

void ULockOnMarkerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	HideMarker();
}

void ULockOnMarkerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateMarkerWorldLocation();
}

void ULockOnMarkerComponent::ShowMarker()
{
	UpdateMarkerWorldLocation();
	
	SetHiddenInGame(false);
	SetVisibility(true, true);
	
	UUserWidget* MarkerWidget = GetWidget();
	
	if (nullptr != MarkerWidget)
	{
		MarkerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	
	SetComponentTickEnabled(true);
}

void ULockOnMarkerComponent::HideMarker()
{
	SetComponentTickEnabled(false);
	
	UUserWidget* MarkerWidget = GetWidget();
	
	if (nullptr != MarkerWidget)
	{
		MarkerWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	SetVisibility(false, true);
	SetHiddenInGame(true);
}

bool ULockOnMarkerComponent::IsMarkerVisible() const
{
	return IsVisible();
}

void ULockOnMarkerComponent::UpdateMarkerWorldLocation()
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	const FVector MarkerWorldLocation = OwnerActor->GetActorLocation() + FVector(0.0f, 0.0f, MarkerHeight);
	
	SetWorldLocation(MarkerWorldLocation);
}
