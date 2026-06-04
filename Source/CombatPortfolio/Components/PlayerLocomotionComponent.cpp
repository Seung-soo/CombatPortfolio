// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerLocomotionComponent.h"
#include "CombatPortfolio/Components/CombatComponent.h"
#include "CombatPortfolio/Components/StaminaComponent.h"
#include "CombatPortfolio/Components/LockOnComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"

UPlayerLocomotionComponent::UPlayerLocomotionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerLocomotionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CacheOwnerComponents();
	ApplyRotationMode();
	UpdateMovementState();
	UpdateMovementSpeed();
}

void UPlayerLocomotionComponent::Move(const FInputActionValue& Value)
{
	if (true == IsMovementBlockedByCombat())
	{
		return;
	}
	
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return;
	}
	
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (false == MovementVector.IsNearlyZero())
	{
		LastMovementInputVector = MovementVector;
	}
	
	AController* Controller = OwnerCharacter->GetController();
	
	if (nullptr == Controller)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	OwnerCharacter->AddMovementInput(ForwardDirection, MovementVector.Y);
	OwnerCharacter->AddMovementInput(RightDirection, MovementVector.X);
}

void UPlayerLocomotionComponent::StopMove()
{
	LastMovementInputVector = FVector2D::ZeroVector;
}

void UPlayerLocomotionComponent::Look(const FInputActionValue& Value)
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return;
	}
	
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	OwnerCharacter->AddControllerYawInput(LookAxisVector.X * LookSensitivityX);
	OwnerCharacter->AddControllerPitchInput(-LookAxisVector.Y * LookSensitivityY);
}

void UPlayerLocomotionComponent::StartWalk()
{
	bWantsToWalk = true;
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

void UPlayerLocomotionComponent::StopWalk()
{
	bWantsToWalk = false;
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

void UPlayerLocomotionComponent::StartSprint()
{
	if (true == IsSprintBlockedByCombat())
	{
		return;
	}
	
	if (nullptr == StaminaComponent)
	{
		return;
	}
	
	if (false == StaminaComponent->HasEnoughStamina(MinStaminaToStartSprint))
	{
		return;
	}
	
	if (false == StaminaComponent->StartStaminaDrain(SprintStaminaDrainRate))
	{
		return;
	}
	
	bWantsToSprint = true;
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

void UPlayerLocomotionComponent::StopSprint(bool bShouldUpdateMovement)
{
	bWantsToSprint = false;
	
	if (nullptr != StaminaComponent)
	{
		StaminaComponent->StopStaminaDrain();
	}
	
	if (true == bShouldUpdateMovement)
	{
		UpdateMovementState();
		UpdateMovementSpeed();
	}
}

void UPlayerLocomotionComponent::HandleCombatActionStarted(EPlayerCombatMovementCleanupMode CleanupMode)
{
	StopSprint(false);
	
	if (EPlayerCombatMovementCleanupMode::StopMoveInput == CleanupMode ||
		EPlayerCombatMovementCleanupMode::StopMovementImmediately == CleanupMode)
	{
		StopMove();
	}
	
	if (EPlayerCombatMovementCleanupMode::StopMovementImmediately == CleanupMode)
	{
		StopMovementImmediately();
	}
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

void UPlayerLocomotionComponent::ToggleRotationMode()
{
	if (ECombatRotationMode::OrientToMovement == RotationMode)
	{
		SetRotationMode(ECombatRotationMode::Strafe);
		return;
	}
	
	SetRotationMode(ECombatRotationMode::OrientToMovement);
}

void UPlayerLocomotionComponent::SetRotationMode(ECombatRotationMode NewRotationMode)
{
	if (RotationMode == NewRotationMode)
	{
		return;
	}
	
	RotationMode = NewRotationMode;
	ApplyRotationMode();
}

void UPlayerLocomotionComponent::ApplyRotationMode()
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return;
	}
	
	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	
	if (nullptr == MovementComponent)
	{
		return;
	}
	
	switch (RotationMode)
	{
	case ECombatRotationMode::OrientToMovement:
		OwnerCharacter->bUseControllerRotationYaw = false;
		MovementComponent->bOrientRotationToMovement = true;
		break;
	case ECombatRotationMode::Strafe:
		OwnerCharacter->bUseControllerRotationYaw = false;
		MovementComponent->bOrientRotationToMovement = false;
		break;
	default:
		OwnerCharacter->bUseControllerRotationYaw = false;
		MovementComponent->bOrientRotationToMovement = true;
		break;
	}
}

void UPlayerLocomotionComponent::UpdateMovementState()
{
	if (true == bWantsToWalk)
	{
		MovementState = ECombatMovementState::Walking;
		return;
	}
	
	if (true == bWantsToSprint)
	{
		MovementState = ECombatMovementState::Sprinting;
		return;
	}
	
	MovementState = ECombatMovementState::Running;
}

void UPlayerLocomotionComponent::UpdateMovementSpeed()
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return;
	}
	
	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	
	if (nullptr == MovementComponent)
	{
		return;
	}
	
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	if (true == CombatComponent->IsDead())
	{
		MovementComponent->MaxWalkSpeed = 0.0f;
		return;
	}
	
	if (true == CombatComponent->IsParrying())
	{
		MovementComponent->MaxWalkSpeed = ParryMoveSpeed;
		return;
	}
		
	if (true == CombatComponent->IsDodging())
	{
		MovementComponent->MaxWalkSpeed = DodgeMoveSpeed;
		return;
	}
		
	if (true == CombatComponent->IsAttacking())
	{
		MovementComponent->MaxWalkSpeed = AttackMoveSpeed;
		return;
	}
	
	switch (MovementState)
	{
	case ECombatMovementState::Walking:
		MovementComponent->MaxWalkSpeed = WalkSpeed;
		break;
	case ECombatMovementState::Running:
		MovementComponent->MaxWalkSpeed = RunSpeed;
		break;
	case ECombatMovementState::Sprinting:
		MovementComponent->MaxWalkSpeed = SprintSpeed;
		break;
	default:
		MovementComponent->MaxWalkSpeed = RunSpeed;
		break;
	}
}

void UPlayerLocomotionComponent::UpdateLockOnRotation(float DeltaSeconds)
{
	if (false == IsLockedOn())
	{
		return;
	}
	
	if (nullptr == LockOnComponent)
	{
		return;
	}
	
	AActor* TargetActor = LockOnComponent->GetLockOnTarget();
	
	if (nullptr == TargetActor)
	{
		return;
	}
	
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return;
	}
	
	const FVector ActorLocation = OwnerCharacter->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	
	FVector ToTarget = TargetLocation - ActorLocation;
	ToTarget.Z = 0.0f;
	
	if (true == ToTarget.IsNearlyZero())
	{
		return;
	}
	
	const FRotator TargetRotation = ToTarget.Rotation();
	const FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
	
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, FRotator(0.0f, TargetRotation.Yaw, 0.0f), DeltaSeconds, LockOnRotationInterpSpeed);
	
	OwnerCharacter->SetActorRotation(NewRotation);
}

FVector UPlayerLocomotionComponent::GetAttackDirection() const
{
	if (true == IsLockedOn())
	{
		const FVector LockOnDirection = GetLockOnAttackDirection();
		
		if (false == LockOnDirection.IsNearlyZero())
		{
			return LockOnDirection;
		}
	}
	
	const FVector MovementInputDirection = GetMovementInputAttackDirection();
	
	if (false == MovementInputDirection.IsNearlyZero())
	{
		return MovementInputDirection;
	}
	
	const FVector CameraForwardDirection = GetCameraForwardAttackDirection();
	
	if (false == CameraForwardDirection.IsNearlyZero())
	{
		return CameraForwardDirection;
	}
	
	const ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	return nullptr != OwnerCharacter ? OwnerCharacter->GetActorForwardVector().GetSafeNormal2D() : FVector::ZeroVector;
}

FVector UPlayerLocomotionComponent::GetDodgeDirection() const
{
	if (true == IsLockedOn())
	{
		return GetLockOnDodgeDirection();
	}
	
	return GetFreeDodgeDirection();
}

void UPlayerLocomotionComponent::FaceDirection(const FVector& Direction)
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return;
	}
	
	const FVector PlanarDirection = Direction.GetSafeNormal2D();
	
	if (true == PlanarDirection.IsNearlyZero())
	{
		return;
	}
	
	const FRotator TargetRotation = PlanarDirection.Rotation();
	
	OwnerCharacter->SetActorRotation(FRotator(0.0f, TargetRotation.Yaw, 0.0f));
}

void UPlayerLocomotionComponent::StopMovementImmediately()
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return;
	}
	
	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	
	if (nullptr != MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
	}
}

bool UPlayerLocomotionComponent::IsLockedOn() const
{
	return nullptr != LockOnComponent && true == LockOnComponent->IsLockedOn();
}

FString UPlayerLocomotionComponent::GetMovementStateDebugString() const
{
	switch (MovementState)
	{
	case ECombatMovementState::Walking:
		return TEXT("Walking");
	case ECombatMovementState::Running:
		return TEXT("Running");
	case ECombatMovementState::Sprinting:
		return TEXT("Sprinting");
	default:
		return TEXT("Unknown");
	}
}

FString UPlayerLocomotionComponent::GetRotationModeDebugString() const
{
	switch (RotationMode)
	{
	case ECombatRotationMode::OrientToMovement:
		return TEXT("OrientToMovement");
	case ECombatRotationMode::Strafe:
		return TEXT("Strafe");
	default:
		return TEXT("Unknown");
	}
}

void UPlayerLocomotionComponent::CacheOwnerComponents()
{
	AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	CombatComponent = OwnerActor->FindComponentByClass<UCombatComponent>();
	StaminaComponent = OwnerActor->FindComponentByClass<UStaminaComponent>();
	LockOnComponent = OwnerActor->FindComponentByClass<ULockOnComponent>();
}

bool UPlayerLocomotionComponent::IsMovementBlockedByCombat() const
{
	if (nullptr == CombatComponent)
	{
		return false;
	}
	
	return CombatComponent->IsDead() || CombatComponent->IsHitReacting() || CombatComponent->IsParrying();
}

bool UPlayerLocomotionComponent::IsSprintBlockedByCombat() const
{
	if (nullptr == CombatComponent)
	{
		return false;
	}
	
	return CombatComponent->IsDead() || CombatComponent->IsHitReacting() || 
			CombatComponent->IsParrying() || CombatComponent->IsAttacking() ||
			CombatComponent->IsDodging();
}

FVector UPlayerLocomotionComponent::GetLockOnAttackDirection() const
{
	return GetPlanarDirectionToLockOnTarget();
}

FVector UPlayerLocomotionComponent::GetMovementInputAttackDirection() const
{
	if (true == LastMovementInputVector.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}
	
	const ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter || nullptr == OwnerCharacter->GetController())
	{
		return FVector::ZeroVector;
	}
	
	const FRotator ControlRotation = OwnerCharacter->GetController()->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	const FVector AttackDirection = ForwardDirection * LastMovementInputVector.Y + RightDirection * LastMovementInputVector.X;
	
	return AttackDirection.GetSafeNormal2D();
}

FVector UPlayerLocomotionComponent::GetCameraForwardAttackDirection() const
{
	const ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter || nullptr == OwnerCharacter->GetController())
	{
		return FVector::ZeroVector;
	}
	
	const FRotator ControlRotation = OwnerCharacter->GetController()->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	
	return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X).GetSafeNormal2D();
}

FVector UPlayerLocomotionComponent::GetFreeDodgeDirection() const
{
	const ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return FVector::ZeroVector;
	}
	
	if (nullptr == OwnerCharacter->GetController())
	{
		return OwnerCharacter->GetActorForwardVector();
	}
	
	const FRotator ControlRotation = OwnerCharacter->GetController()->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	FVector DodgeDirection = ForwardDirection * LastMovementInputVector.Y + RightDirection * LastMovementInputVector.X;
	
	if (true == DodgeDirection.IsNearlyZero())
	{
		DodgeDirection = OwnerCharacter->GetActorForwardVector();
	}
	
	return DodgeDirection.GetSafeNormal();
}

FVector UPlayerLocomotionComponent::GetLockOnDodgeDirection() const
{
	const ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return FVector::ZeroVector;
	}
	
	const FVector ForwardToTarget = GetPlanarDirectionToLockOnTarget();
	
	if (true == ForwardToTarget.IsNearlyZero())
	{
		return OwnerCharacter->GetActorForwardVector();
	}
	
	const FVector RightToTarget = FVector::CrossProduct(FVector::UpVector, ForwardToTarget).GetSafeNormal();
	
	FVector DodgeDirection = ForwardToTarget * LastMovementInputVector.Y + RightToTarget * LastMovementInputVector.X;
	
	if (true == DodgeDirection.IsNearlyZero())
	{
		DodgeDirection = -ForwardToTarget;
	}
	
	return DodgeDirection.GetSafeNormal();
}

FVector UPlayerLocomotionComponent::GetPlanarDirectionToLockOnTarget() const
{
	if (nullptr == LockOnComponent)
	{
		
		return FVector::ZeroVector;
	}
	
	AActor* TargetActor = LockOnComponent->GetLockOnTarget();
	
	if (nullptr == TargetActor)
	{
		return FVector::ZeroVector;
	}
	
	const ACharacter* OwnerCharacter = GetOwnerCharacter();
	
	if (nullptr == OwnerCharacter)
	{
		return FVector::ZeroVector;
	}
	
	FVector ToTarget = TargetActor->GetActorLocation() - OwnerCharacter->GetActorLocation();
	ToTarget.Z = 0.0f;
	
	return ToTarget.GetSafeNormal();
}

ACharacter* UPlayerLocomotionComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}