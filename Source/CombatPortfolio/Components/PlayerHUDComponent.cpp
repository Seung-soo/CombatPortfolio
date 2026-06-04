// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDComponent.h"

#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "CombatPortfolio/CombatPortfolio.h"
#include "CombatPortfolio/UI/PlayerHUDWidget.h"

UPlayerHUDComponent::UPlayerHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerHUDComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CacheOwnerComponents();
	CreatePlayerHUD();
	InitializeHUD();
	
	if (nullptr != HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &UPlayerHUDComponent::HandleHealthChanged);
	}
	
	if (nullptr != StaminaComponent)
	{
		StaminaComponent->OnStaminaChanged.AddDynamic(this, &UPlayerHUDComponent::HandleStaminaChanged);
	}
}

void UPlayerHUDComponent::InitializeHUD()
{
	if (nullptr == PlayerHUDWidget)
	{
		return;
	}
	
	if (nullptr == HealthComponent || nullptr == StaminaComponent)
	{
		return;
	}
	
	PlayerHUDWidget->InitializeHUD(
		HealthComponent->GetCurrentHealth(),
		HealthComponent->GetMaxHealth(),
		StaminaComponent->GetCurrentStamina(),
		StaminaComponent->GetMaxStamina()
	);
}

void UPlayerHUDComponent::ShowDeathMessage()
{
	if (nullptr == PlayerHUDWidget)
	{
		return;
	}
	
	PlayerHUDWidget->ShowDeathMessage();
}

void UPlayerHUDComponent::CacheOwnerComponents()
{
	AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	HealthComponent = OwnerActor->FindComponentByClass<UHealthComponent>();
	StaminaComponent = OwnerActor->FindComponentByClass<UStaminaComponent>();
}

void UPlayerHUDComponent::CreatePlayerHUD()
{
	if (nullptr == PlayerHUDWidgetClass)
	{
		UE_LOG(LogCombatPortfolio, Warning, TEXT("PlayerHUDWidgetClass is not assigned."));
		return;
	}
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	
	if (nullptr == OwnerPawn)
	{
		return;
	}
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	
	if (nullptr == PlayerController)
	{
		return;
	}
	
	PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(PlayerController, PlayerHUDWidgetClass);
	
	if (nullptr == PlayerHUDWidget)
	{
		return;
	}
	
	PlayerHUDWidget->AddToViewport();
}

void UPlayerHUDComponent::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
	if (nullptr == PlayerHUDWidget)
	{
		return;
	}
	
	PlayerHUDWidget->SetHealth(CurrentHealth, MaxHealth);
}

void UPlayerHUDComponent::HandleStaminaChanged(float CurrentStamina, float MaxStamina, float Delta)
{
	if (nullptr == PlayerHUDWidget)
	{
		return;
	}
	
	PlayerHUDWidget->SetStamina(CurrentStamina, MaxStamina);
}