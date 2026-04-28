// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPlayerController.h"
#include "EnhancedInputSubsystems.h"

void ACombatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LocalPlayer = GetLocalPlayer();

	if (nullptr == LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (nullptr == InputSubsystem)
	{
		return;
	}

	if (nullptr != DefaultMappingContext)
	{
		InputSubsystem->AddMappingContext(DefaultMappingContext, MappingPriority);
	}
}
