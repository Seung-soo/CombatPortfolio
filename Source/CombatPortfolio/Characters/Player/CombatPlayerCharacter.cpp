// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "CombatPortfolio/Components/CombatComponent.h"
#include "CombatPortfolio/Components/StaminaComponent.h"
#include "CombatPortfolio/Components/HealthComponent.h"
#include "CombatPortfolio/Components/LockOnComponent.h"
#include "DrawDebugHelpers.h"
#include "CombatPortfolio/UI/PlayerHUDWidget.h"

// Sets default values
ACombatPlayerCharacter::ACombatPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));
}

void ACombatPlayerCharacter::DebugApplyDamageToPlayer(float DamageAmount)
{
	if (nullptr == HealthComponent)
	{
		return;
	}
	
	if (nullptr != CombatComponent && true == CombatComponent->IsInvincible())
	{
		UE_LOG(LogTemp, Log, TEXT("Debug damage blocked: Player is invincible"));
		
		if (nullptr != GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Cyan, TEXT("Damage Blocked by IFrame"));
		}
		
		return;
	}
	
	HealthComponent->ApplyDamage(DamageAmount);
}

// Called when the game starts or when spawned
void ACombatPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (nullptr != CombatComponent)
	{
		CombatComponent->OnCombatActionStateChanged.AddDynamic(this, &ACombatPlayerCharacter::HandleCombatActionStateChanged);
	}
	
	if (nullptr != StaminaComponent)
	{
		StaminaComponent->OnStaminaDepleted.AddDynamic(this, &ACombatPlayerCharacter::HandleStaminaDepleted);
		StaminaComponent->OnStaminaChanged.AddDynamic(this, &ACombatPlayerCharacter::HandleStaminaChanged);
	}
	
	if (nullptr != HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ACombatPlayerCharacter::HandleHealthChanged);
		HealthComponent->OnDeath.AddDynamic(this, &ACombatPlayerCharacter::HandleDeath);
	}
	
	if (nullptr != LockOnComponent)
	{
		LockOnComponent->OnLockOnTargetChaged.AddDynamic(this, &ACombatPlayerCharacter::HandleLockOnTargetChanged);
	}
	
	CreatePlayerHUD();
	InitializePlayerHUD();
	
	ApplyRotationMode();
	UpdateMovementState();
	UpdateMovementSpeed();
	
	UpdateCharacterTickEnabled();
}

void ACombatPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLockOnRotation(DeltaTime);
	PrintMovementDebug();
}

// Called to bind functionality to input
void ACombatPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (nullptr == EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerInputComponent is not EnhancedInputComponent"));
		return;
	}

	if (nullptr != MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACombatPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACombatPlayerCharacter::StopMove);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ACombatPlayerCharacter::StopMove);
	}

	if (nullptr != LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACombatPlayerCharacter::Look);
	}
	
	if (nullptr != WalkAction)
	{
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::StartWalk);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &ACombatPlayerCharacter::StopWalk);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Canceled, this, &ACombatPlayerCharacter::StopWalk);
	}
	
	if (nullptr != SprintAction)
	{
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACombatPlayerCharacter::StopSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ACombatPlayerCharacter::StopSprint);
	}
	
	if (nullptr != ToggleRotationModeAction)
	{
		EnhancedInputComponent->BindAction(ToggleRotationModeAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::ToggleRotationMode);
	}
	
	if (nullptr != AttackAction)
	{
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::Attack);
	}
	
	if (nullptr != DodgeAction)
	{
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::Dodge);
	}
	
	if (nullptr != LockOnAction)
	{
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::ToggleLockOn);
	}
}

void ACombatPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (nullptr != CombatComponent && true == CombatComponent->IsHitReacting())
	{
		return;
	}
	
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (false == MovementVector.IsNearlyZero())
	{
		LastMovementInputVector = MovementVector;
	}

	if (nullptr == Controller)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ACombatPlayerCharacter::StopMove()
{
	LastMovementInputVector = FVector2D::ZeroVector;
}

void ACombatPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X * LookSensitivityX);
	AddControllerPitchInput(-LookAxisVector.Y * LookSensitivityY);
}

void ACombatPlayerCharacter::StartWalk()
{
	bWantsToWalk = true;
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

void ACombatPlayerCharacter::StopWalk()
{
	bWantsToWalk = false;
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

void ACombatPlayerCharacter::StartSprint()
{
	if (nullptr != CombatComponent && true == CombatComponent->IsHitReacting())
	{
		return;
	}
	
	if (true == IsCombatAttacking() || true == IsCombatDodging())
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

void ACombatPlayerCharacter::StopSprint()
{
	bWantsToSprint = false;
	
	if (nullptr != StaminaComponent)
	{
		StaminaComponent->StopStaminaDrain();
	}
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

void ACombatPlayerCharacter::ToggleRotationMode()
{
	if (ECombatRotationMode::OrientToMovement == RotationMode)
	{
		SetRotationMode(ECombatRotationMode::Strafe);
		return;
	}
	
	SetRotationMode(ECombatRotationMode::OrientToMovement);
}

void ACombatPlayerCharacter::SetRotationMode(ECombatRotationMode NewRotationMode)
{
	if (RotationMode == NewRotationMode)
	{
		return;
	}
	
	RotationMode = NewRotationMode;
	ApplyRotationMode();
}

void ACombatPlayerCharacter::ApplyRotationMode()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	
	if (nullptr == MovementComponent)
	{
		return;
	}
	
	switch (RotationMode)
	{
	case ECombatRotationMode::OrientToMovement:
		bUseControllerRotationYaw = false;
		MovementComponent->bOrientRotationToMovement = true;
		break;
	case ECombatRotationMode::Strafe:
		bUseControllerRotationYaw = false;
		MovementComponent->bOrientRotationToMovement = false;
		break;
	default:
		bUseControllerRotationYaw = false;
		MovementComponent->bOrientRotationToMovement = true;
		break;
	}
}

void ACombatPlayerCharacter::UpdateMovementState()
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

void ACombatPlayerCharacter::UpdateMovementSpeed()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	
	if (nullptr == MovementComponent)
	{
		return;
	}
	
	if (true == IsCombatDodging())
	{
		MovementComponent->MaxWalkSpeed = DodgeMoveSpeed;
		return;
	}
	
	if (true == IsCombatAttacking())
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

void ACombatPlayerCharacter::Attack()
{
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	const bool bAttackStarted = CombatComponent->RequestAttack();
	
	if (false == bAttackStarted)
	{
		return;
	}
	
	bWantsToSprint = false;
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

void ACombatPlayerCharacter::Dodge()
{
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	if (nullptr == StaminaComponent)
	{
		return;
	}
	
	if (false == CombatComponent->CanStartDodge())
	{
		return;
	}
	
	if (false == StaminaComponent->TrySpendStamina(DodgeStaminaCost))
	{
		return;
	}
	
	const FVector DodgeDirection = GetDodgeDirection();
	
	DrawDodgeDirectionDebug(DodgeDirection);
	
	if (false == DodgeDirection.IsNearlyZero())
	{
		const FRotator DodgeRotation = DodgeDirection.Rotation();
		SetActorRotation(FRotator(0.0f, DodgeRotation.Yaw, 0.0f));
	}
	
	const bool bDodgeStarted = CombatComponent->RequestDodge(DodgeDirection);
	
	if (false == bDodgeStarted)
	{
		return;
	}
	
	bWantsToSprint = false;
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

bool ACombatPlayerCharacter::IsCombatDodging() const
{
	return nullptr != CombatComponent && true == CombatComponent->IsDodging();
}

void ACombatPlayerCharacter::ToggleLockOn()
{
	if (nullptr == LockOnComponent)
	{
		return;
	}
	
	LockOnComponent->ToggleLockOn();
}

void ACombatPlayerCharacter::UpdateLockOnRotation(float DeltaSeconds)
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
	
	const FVector ActorLocation = GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	
	FVector ToTarget = TargetLocation - ActorLocation;
	ToTarget.Z = 0.0f;
	
	if (true == ToTarget.IsNearlyZero())
	{
		return;
	}
	
	const FRotator TargetRotation = ToTarget.Rotation();
	const FRotator CurrentRotation = GetActorRotation();
	
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, FRotator(0.0f, TargetRotation.Yaw, 0.0f), DeltaSeconds, LockOnRotationInterpSpeed);
	
	SetActorRotation(NewRotation);
}

bool ACombatPlayerCharacter::IsLockedOn() const
{
	return nullptr != LockOnComponent && true == LockOnComponent->IsLockedOn();
}

void ACombatPlayerCharacter::UpdateCharacterTickEnabled()
{
	const bool bShouldTick = true == bShowMovementDebug || true == IsLockedOn();
	
	SetActorTickEnabled(bShouldTick);
}

void ACombatPlayerCharacter::CreatePlayerHUD()
{
	if (nullptr == PlayerHUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerHUDWidgetClass is not assigned."));
		return;
	}
	
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	
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

void ACombatPlayerCharacter::InitializePlayerHUD()
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

FVector ACombatPlayerCharacter::GetDodgeDirection() const
{
	if (true == IsLockedOn())
	{
		return GetLockOnDodgeDirection();
	}
	
	return GetFreeDodgeDirection();
}

FVector ACombatPlayerCharacter::GetFreeDodgeDirection() const
{
	if (nullptr == Controller)
	{
		return GetActorForwardVector();
	}
	
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	FVector DodgeDirection = ForwardDirection * LastMovementInputVector.Y + RightDirection * LastMovementInputVector.X;
	
	if (true == DodgeDirection.IsNearlyZero())
	{
		DodgeDirection = GetActorForwardVector();
	}
	
	return DodgeDirection.GetSafeNormal();
}

FVector ACombatPlayerCharacter::GetLockOnDodgeDirection() const
{
	const FVector ForwardToTarget = GetPlanarDirectionToLockOnTarget();
	
	if (true == ForwardToTarget.IsNearlyZero())
	{
		return GetActorForwardVector();
	}
	
	const FVector RightToTarget = FVector::CrossProduct(FVector::UpVector, ForwardToTarget).GetSafeNormal();
	
	FVector DodgeDirection = ForwardToTarget * LastMovementInputVector.Y + RightToTarget * LastMovementInputVector.X;
	
	if (true == DodgeDirection.IsNearlyZero())
	{
		DodgeDirection = -ForwardToTarget;
	}
	
	return DodgeDirection.GetSafeNormal();
}

FVector ACombatPlayerCharacter::GetPlanarDirectionToLockOnTarget() const
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
	
	FVector ToTarget = TargetActor->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	
	return ToTarget.GetSafeNormal();
}

FString ACombatPlayerCharacter::GetInvincibilityDebugString() const
{
	if (nullptr == CombatComponent)
	{
		return TEXT("None");
	}
	
	return true == CombatComponent->IsInvincible() ? TEXT("Invincible") : TEXT("Vulnerable");
}

FString ACombatPlayerCharacter::GetLockOnDebugString() const
{
	if (nullptr == LockOnComponent)
	{
		return TEXT("None");
	}
	
	AActor* TargetActor = LockOnComponent->GetLockOnTarget();
	
	if (nullptr == TargetActor)
	{
		return TEXT("Unlocked");
	}
	
	return TargetActor->GetName();
}

bool ACombatPlayerCharacter::IsCombatAttacking() const
{
	return nullptr != CombatComponent && true == CombatComponent->IsAttacking();
}

void ACombatPlayerCharacter::HandleCombatActionStateChanged()
{
	UpdateMovementState();
	UpdateMovementSpeed();
}

void ACombatPlayerCharacter::HandleStaminaDepleted()
{
	bWantsToSprint = false;
	
	UpdateMovementState();
	UpdateMovementSpeed();
}

void ACombatPlayerCharacter::HandleStaminaChanged(float CurrentStamina, float MaxStamina, float Delta)
{
	if (nullptr != PlayerHUDWidget)
	{
		PlayerHUDWidget->SetStamina(CurrentStamina, MaxStamina);
	}
}

void ACombatPlayerCharacter::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
	if (nullptr != PlayerHUDWidget)
	{
		PlayerHUDWidget->SetHealth(CurrentHealth, MaxHealth);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Player Health Changed: %.1f / %.1f | Delta: %.1f"), CurrentHealth, MaxHealth, Delta);
	
	if (0.0f > Delta && 0.0f < CurrentHealth)
	{
		if (nullptr != CombatComponent)
		{
			const bool bHitReactionStarted = CombatComponent->RequestHitReaction();
			
			if (true == bHitReactionStarted)
			{
				bWantsToSprint = false;
				UpdateMovementState();
				UpdateMovementSpeed();
			}
		}
	}
	
	if (0.0f > Delta && nullptr != GEngine)
	{
		const FString DebugText = FString::Printf(TEXT("Player Hit! HP: %.1f / %.1f"), CurrentHealth, MaxHealth);
		
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, DebugText);
	}
}

void ACombatPlayerCharacter::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Player died"));
	
	if (nullptr != GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Player Dead"));
	}
	
	DisableInput(nullptr);
	
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	
	if (nullptr != MovementComponent)
	{
		MovementComponent->DisableMovement();
	}
}

void ACombatPlayerCharacter::HandleLockOnTargetChanged()
{
	if (true == IsLockedOn())
	{
		SetRotationMode(ECombatRotationMode::Strafe);
	}
	else
	{
		SetRotationMode(ECombatRotationMode::OrientToMovement);
	}
	
	UpdateCharacterTickEnabled();
}

FString ACombatPlayerCharacter::GetStaminaDebugString() const
{
	if (nullptr == StaminaComponent)
	{
		return TEXT("None");
	}
	
	const FString DrainState = StaminaComponent->IsDrainingStamina() ? TEXT("Drain") : TEXT("NoDrain");
	
	return FString::Printf(TEXT("%.1f / %.1f | %s"), StaminaComponent->GetCurrentStamina(), StaminaComponent->GetMaxStamina(), *DrainState);
}

FString ACombatPlayerCharacter::GetHealthDebugString() const
{
	if (nullptr == HealthComponent)
	{
		return TEXT("None");
	}
	
	return FString::Printf(TEXT("%.1f / %.1f"), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
}

FString ACombatPlayerCharacter::GetCombatStateDebugString() const
{
	if (nullptr == CombatComponent)
	{
		return TEXT("None");
	}
	
	switch (CombatComponent->GetCombatActionState())
	{
	case ECombatActionState::Idle:
		return TEXT("Idle");
	case ECombatActionState::Attacking:
		return TEXT("Attacking");
	case ECombatActionState::Dodging:
		return TEXT("Dodging");
	case ECombatActionState::HitReaction:
		return TEXT("HitReaction");
	default:
		return TEXT("Unknown");
	}
}

FString ACombatPlayerCharacter::GetHitWindowDebugString() const
{
	if (nullptr == CombatComponent)
	{
		return TEXT("None");
	}
	
	return true == CombatComponent->IsHitWindowOpen() ? TEXT("Open") : TEXT("Closed");
}

FString ACombatPlayerCharacter::GetComboDebugString() const
{
	if (nullptr == CombatComponent)
	{
		return TEXT("None");
	}
	
	const int32 DisplayComboIndex = CombatComponent->GetCurrentComboIndex() + 1;
	const FString WindowState = true == CombatComponent->IsComboInputWindowOpen() ? TEXT("Open") : TEXT("Closed");
	
	const FString BufferedState = true == CombatComponent->HasBufferedComboInput() ? TEXT("Buffered") : TEXT("None");
	
	return FString::Printf(TEXT("%d | Damage: %.1f | Window: %s | Buffer: %s"), 
		DisplayComboIndex, CombatComponent->GetCurrentAttackDamage(), *WindowState, *BufferedState);
}

int32 ACombatPlayerCharacter::GetHitActorCountDebug() const
{
	if (nullptr == CombatComponent)
	{
		return 0;
	}
	
	return CombatComponent->GetHitActorCountThisAttack();
}

void ACombatPlayerCharacter::PrintMovementDebug() const
{
	if (false == bShowMovementDebug)
	{
		return;
	}
	
	if (nullptr == GEngine)
	{
		return;
	}
	
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	
	if (nullptr == MovementComponent)
	{
		return;
	}
	
	FString MovementStateString = TEXT("Unknown");
	
	switch (MovementState)
	{
	case ECombatMovementState::Walking:
		MovementStateString = TEXT("Walking");
		break;
	case ECombatMovementState::Running:
		MovementStateString = TEXT("Running");
		break;
	case ECombatMovementState::Sprinting:
		MovementStateString = TEXT("Sprinting");
		break;
	default:
		break;
	}
	
	FString RotationModeString = TEXT("Unknown");
	
	switch (RotationMode)
	{
	case ECombatRotationMode::OrientToMovement:
		RotationModeString = TEXT("OrientToMovement");
		break;
	case ECombatRotationMode::Strafe:
		RotationModeString = TEXT("Strafe");
		break;
	default:
		break;
	}
	
	const FVector Velocity = GetVelocity();
	const float GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
	
	const float ControlYaw = nullptr != Controller ? Controller->GetControlRotation().Yaw : 0.0f;
	
	const FString DebugText = FString::Printf(
		TEXT("MovementState: %s | RotationMode: %s | LockOn: %s | CombatState: %s | HP: %s | IFrame: %s | Stamina: %s | Combo: %s | HitWindow: %s | HitCount: %d | GroundSpeed: %.1f | MaxWalkSpeed: %.1f | ControlYaw: %.1f"),
		*MovementStateString,
		*RotationModeString,
		*GetLockOnDebugString(),
		*GetCombatStateDebugString(),
		*GetHealthDebugString(),
		*GetInvincibilityDebugString(),
		*GetStaminaDebugString(),
		*GetComboDebugString(),
		*GetHitWindowDebugString(),
		GetHitActorCountDebug(),
		GroundSpeed,
		MovementComponent->MaxWalkSpeed,
		ControlYaw
	);
	
	GEngine->AddOnScreenDebugMessage(
		1,
		0.0f,
		FColor::Green,
		DebugText
	);
}

void ACombatPlayerCharacter::DrawDodgeDirectionDebug(const FVector& DodgeDirection) const
{
	if (false == bDrawDodgeDirectionDebug)
	{
		return;
	}
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	if (true == DodgeDirection.IsNearlyZero())
	{
		return;
	}
	
	const FVector StartLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const FVector EndLocation = StartLocation + DodgeDirection.GetSafeNormal() * DodgeDirectionDebugLength;
	
	DrawDebugDirectionalArrow(
		World,
		StartLocation,
		EndLocation,
		60.0f,
		FColor::Cyan,
		false,
		1.0f,
		0,
		4.0f
	);
}

