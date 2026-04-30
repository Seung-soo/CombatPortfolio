// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "CombatPortfolio/Components/CombatComponent.h"

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
}

// Called when the game starts or when spawned
void ACombatPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (nullptr != CombatComponent)
	{
		CombatComponent->OnCombatActionStateChanged.AddDynamic(this, &ACombatPlayerCharacter::HandleCombatActionStateChanged);
	}
	
	ApplyRotationMode();
	UpdateMovementState();
	UpdateMovementSpeed();
	
	SetActorTickEnabled(bShowMovementDebug);
}

void ACombatPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
}

void ACombatPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

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
	if (true == IsCombatAttacking())
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
		bUseControllerRotationYaw = true;
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

bool ACombatPlayerCharacter::IsCombatAttacking() const
{
	return nullptr != CombatComponent && true == CombatComponent->IsAttacking();
}

void ACombatPlayerCharacter::HandleCombatActionStateChanged()
{
	UpdateMovementState();
	UpdateMovementSpeed();
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
	default:
		return TEXT("Unknown");
	}
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
		TEXT("MovementState: %s | RotationMode: %s | CombatState: %s | GroundSpeed: %.1f | MaxWalkSpeed: %.1f | ControlYaw: %.1f"),
		*MovementStateString,
		*RotationModeString,
		*GetCombatStateDebugString(),
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

