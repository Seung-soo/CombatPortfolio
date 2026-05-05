// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthBarComponent.h"

#include "CombatPortfolio/UI/EnemyHealthBarWidget.h"

UEnemyHealthBarComponent::UEnemyHealthBarComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawSize(FVector2D(140.0f, 24.0f));
	SetPivot(FVector2D(0.5f, 0.5f));
	
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	
	SetVisibility(false, true);
	SetHiddenInGame(true);
}

void UEnemyHealthBarComponent::BeginPlay()
{
	Super::BeginPlay();
	
	HideHealthBar();
}

void UEnemyHealthBarComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateHealthBarWorldLocation();
}

void UEnemyHealthBarComponent::InitializeHealth(float CurrentHealth, float MaxHealth)
{
	SetHealth(CurrentHealth, MaxHealth);
}

void UEnemyHealthBarComponent::SetHealth(float CurrentHealth, float MaxHealth)
{
	UEnemyHealthBarWidget* HealthBarWidget = GetEnemyHealthBarWidget();
	
	if (nullptr != HealthBarWidget)
	{
		HealthBarWidget->SetHealth(CurrentHealth, MaxHealth);
	}
	
	const bool bFullHealth = 0.0f < MaxHealth && true == FMath::IsNearlyEqual(CurrentHealth, MaxHealth);
	
	if (true == bHideWhenFullHealth && true == bFullHealth)
	{
		HideHealthBar();
		return;
	}
	
	if (0.0f < CurrentHealth)
	{
		ShowHealthBar();
	}
	else
	{
		HideHealthBar();
	}
}

void UEnemyHealthBarComponent::ShowHealthBar()
{
	UpdateHealthBarWorldLocation();
	
	SetHiddenInGame(false);
	SetVisibility(true, true);
	
	UUserWidget* HealthBarWidget = GetUserWidgetObject();
	
	if (nullptr != HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	
	SetComponentTickEnabled(true);
}

void UEnemyHealthBarComponent::HideHealthBar()
{
	SetComponentTickEnabled(false);
	
	UUserWidget* HealthBarWidget = GetUserWidgetObject();
	
	if (nullptr != HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	SetVisibility(false, true);
	SetHiddenInGame(true);
}

void UEnemyHealthBarComponent::UpdateHealthBarWorldLocation()
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	const FVector HealthBarWorldLocation = OwnerActor->GetActorLocation() + FVector(0.0f, 0.0f, HealthBarHeight);
	
	SetWorldLocation(HealthBarWorldLocation);
}

UEnemyHealthBarWidget* UEnemyHealthBarComponent::GetEnemyHealthBarWidget() const
{
	return Cast<UEnemyHealthBarWidget>(GetUserWidgetObject());
}
