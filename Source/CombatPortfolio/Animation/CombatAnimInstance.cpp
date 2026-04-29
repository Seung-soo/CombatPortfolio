// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCombatAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	OwnerPawn = TryGetPawnOwner();
}

void UCombatAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (nullptr == OwnerPawn)
	{
		OwnerPawn = TryGetPawnOwner();
	}
	
	if (nullptr == OwnerPawn)
	{
		return;
	}
	
	const FVector Velocity = OwnerPawn->GetVelocity();
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
	
	const ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn);
	
	if (nullptr == OwnerCharacter)
	{
		bIsFalling = false;
		return;
	}
	
	const UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	
	if (nullptr == MovementComponent)
	{
		bIsFalling = false;
		return;
	}
	
	bIsFalling = MovementComponent->IsFalling();
}
