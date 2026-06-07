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
#include "CombatPortfolio/CombatPortfolio.h"
#include "CombatPortfolio/Characters/Enemy/CombatEnemyBase.h"
#include "CombatPortfolio/Combat/CombatDamageLibrary.h"
#include "CombatPortfolio/Components/HitStopComponent.h"
#include "CombatPortfolio/Components/PlayerAttackComponent.h"
#include "CombatPortfolio/Components/PlayerDefenseComponent.h"
#include "CombatPortfolio/Components/PlayerHUDComponent.h"
#include "CombatPortfolio/Components/PlayerLocomotionComponent.h"
#include "CombatPortfolio/Components/PlayerReactionComponent.h"

ACombatPlayerCharacter::ACombatPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
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
	
	PlayerAttackComponent = CreateDefaultSubobject<UPlayerAttackComponent>(TEXT("PlayerAttackComponent"));
	
	PlayerDefenseComponent = CreateDefaultSubobject<UPlayerDefenseComponent>(TEXT("PlayerDefenseComponent"));
	
	PlayerReactionComponent = CreateDefaultSubobject<UPlayerReactionComponent>(TEXT("PlayerReactionComponent"));
	
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));
	
	HitStopComponent = CreateDefaultSubobject<UHitStopComponent>(TEXT("HitStopComponent"));
	
	PlayerHUDComponent = CreateDefaultSubobject<UPlayerHUDComponent>(TEXT("PlayerHUDComponent"));
	
	PlayerLocomotionComponent = CreateDefaultSubobject<UPlayerLocomotionComponent>(TEXT("PlayerLocomotionComponent"));
}

void ACombatPlayerCharacter::DebugApplyDamageToPlayer(float DamageAmount)
{
	if (nullptr == HealthComponent)
	{
		return;
	}
	
	if (nullptr != CombatComponent && true == CombatComponent->IsInvincible())
	{
		UE_LOG(LogCombatPortfolio, Log, TEXT("Debug damage blocked: Player is invincible"));
		
		if (nullptr != GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Cyan, TEXT("Damage Blocked by IFrame"));
		}
		
		return;
	}
	
	HealthComponent->ApplyDamage(DamageAmount);
}

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
	}
	
	if (nullptr != HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ACombatPlayerCharacter::HandleHealthChanged);
		HealthComponent->OnDamaged.AddDynamic(this, &ACombatPlayerCharacter::HandleDamaged);
		HealthComponent->OnDeath.AddDynamic(this, &ACombatPlayerCharacter::HandleDeath);
	}
	
	if (nullptr != LockOnComponent)
	{
		LockOnComponent->OnLockOnTargetChanged.AddDynamic(this, &ACombatPlayerCharacter::HandleLockOnTargetChanged);
	}

	UpdateCharacterTickEnabled();
}

void ACombatPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->UpdateLockOnRotation(DeltaTime);
	}

	PrintMovementDebug();
}

void ACombatPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (nullptr == EnhancedInputComponent)
	{
		UE_LOG(LogCombatPortfolio, Error, TEXT("PlayerInputComponent is not EnhancedInputComponent"));
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
	
	if (nullptr != LightAttackAction)
	{
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::Attack);
	}
	
	if (nullptr != DodgeAction)
	{
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::Dodge);
	}
	
	if (nullptr != LockOnAction)
	{
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::ToggleLockOn);
	}
	
	if (nullptr != HeavyAttackAction)
	{
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::HeavyAttack);
	}
	
	if (nullptr != ParryAction)
	{
		EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Started, this, &ACombatPlayerCharacter::Parry);
	}
}

void ACombatPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->Move(Value);
	}
}

void ACombatPlayerCharacter::StopMove()
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->StopMove();
	}
}

void ACombatPlayerCharacter::Look(const FInputActionValue& Value)
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->Look(Value);
	}
}

void ACombatPlayerCharacter::StartWalk()
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->StartWalk();
	}
}

void ACombatPlayerCharacter::StopWalk()
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->StopWalk();
	}
}

void ACombatPlayerCharacter::StartSprint()
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->StartSprint();
	}
}

void ACombatPlayerCharacter::StopSprint()
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->StopSprint();
	}
}

void ACombatPlayerCharacter::ToggleRotationMode()
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->ToggleRotationMode();
	}
}


void ACombatPlayerCharacter::Attack()
{
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	if (true == CombatComponent->IsDead())
	{
		return;
	}
	
	ACombatEnemyBase* CriticalTarget = GetCriticalAttackTarget();
	
	if (nullptr != CriticalTarget)
	{
		if (nullptr != PlayerLocomotionComponent && true == CombatComponent->CanStartAttack())
		{
			const FVector CriticalDirection = (CriticalTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
			PlayerLocomotionComponent->FaceDirection(CriticalDirection);
		}
		
		const bool bCriticalAttackStarted = CombatComponent->RequestCriticalAttack(CriticalTarget);
		
		if (false == bCriticalAttackStarted)
		{
			return;
		}
		
		if (nullptr != PlayerLocomotionComponent)
		{
			PlayerLocomotionComponent->HandleCombatActionStarted(EPlayerCombatMovementCleanupMode::StopSprintOnly);
		}
		
		return;
	}
	
	if (nullptr != PlayerLocomotionComponent && true == CombatComponent->CanStartAttack())
	{
		const FVector AttackDirection = PlayerLocomotionComponent->GetAttackDirection();
		PlayerLocomotionComponent->FaceDirection(AttackDirection);
	}
	
	const bool bAttackStarted = CombatComponent->RequestAttack(ECombatAttackInputType::Light);
	
	if (false == bAttackStarted)
	{
		return;
	}
	
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->HandleCombatActionStarted(EPlayerCombatMovementCleanupMode::StopSprintOnly);
	}
}

void ACombatPlayerCharacter::Dodge()
{
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	if (true == CombatComponent->IsDead())
	{
		return;
	}

	if (false == CombatComponent->CanStartDodge())
	{
		return;
	}
	
	const FVector DodgeDirection = nullptr != PlayerLocomotionComponent ? 
	PlayerLocomotionComponent->GetDodgeDirection() : GetActorForwardVector();
	
	DrawDodgeDirectionDebug(DodgeDirection);
	
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->FaceDirection(DodgeDirection);
	}
	
	const bool bDodgeStarted = CombatComponent->RequestDodge(DodgeDirection);
	
	if (false == bDodgeStarted)
	{
		return;
	}

	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->HandleCombatActionStarted(EPlayerCombatMovementCleanupMode::StopSprintOnly);
	}
}

void ACombatPlayerCharacter::HeavyAttack()
{
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	if (true == CombatComponent->IsDead())
	{
		return;
	}
	
	if (nullptr != PlayerLocomotionComponent && true == CombatComponent->CanStartAttack())
	{
		const FVector AttackDirection = PlayerLocomotionComponent->GetAttackDirection();
		PlayerLocomotionComponent->FaceDirection(AttackDirection);
	}
	
	const bool bAttackStarted = CombatComponent->RequestAttack(ECombatAttackInputType::Heavy);
	
	if (false == bAttackStarted)
	{
		return;
	}
	
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->HandleCombatActionStarted(EPlayerCombatMovementCleanupMode::StopSprintOnly);
	}
}

void ACombatPlayerCharacter::Parry()
{
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	if (true == CombatComponent->IsDead())
	{
		return;
	}
	
	const bool bParryStarted = CombatComponent->RequestParry();
	
	if (false == bParryStarted)
	{
		return;
	}
	
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->HandleCombatActionStarted(EPlayerCombatMovementCleanupMode::StopMovementImmediately);
	}
}

void ACombatPlayerCharacter::ToggleLockOn()
{
	if (nullptr == LockOnComponent)
	{
		return;
	}
	
	LockOnComponent->ToggleLockOn();
}

void ACombatPlayerCharacter::UpdateCharacterTickEnabled()
{
	const bool bShouldTick = true == bShowMovementDebug || (nullptr != PlayerLocomotionComponent && true == PlayerLocomotionComponent->IsLockedOn());
	
	SetActorTickEnabled(bShouldTick);
}

ACombatEnemyBase* ACombatPlayerCharacter::GetCriticalAttackTarget() const
{
	if (nullptr == LockOnComponent)
	{
		return nullptr;
	}
	
	ACombatEnemyBase* LockedEnemy = Cast<ACombatEnemyBase>(LockOnComponent->GetLockOnTarget());
	
	if (nullptr == LockedEnemy)
	{
		return nullptr;
	}
	
	if (false == LockedEnemy->IsCriticalAttackAvailable())
	{
		return nullptr;
	}
	
	return LockedEnemy;
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

void ACombatPlayerCharacter::HandleCombatActionStateChanged()
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->UpdateMovementState();
		PlayerLocomotionComponent->UpdateMovementSpeed();
	}
}

void ACombatPlayerCharacter::HandleStaminaDepleted()
{
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->StopSprint();
	}
}

void ACombatPlayerCharacter::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
	UE_LOG(LogCombatPortfolio, Log, TEXT("Player Health Changed: %.1f / %.1f | Delta: %.1f"), CurrentHealth, MaxHealth, Delta);
	
	if (0.0f > Delta && nullptr != GEngine)
	{
		const FString DebugText = FString::Printf(TEXT("Player Hit! HP: %.1f / %.1f"), CurrentHealth, MaxHealth);
		
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, DebugText);
	}
}

void ACombatPlayerCharacter::HandleDamaged(const FCombatDamageInfo& DamageInfo)
{
	UCombatDamageLibrary::ApplyDamageFeedbackFromDamageInfo(DamageInfo);
	
	if (nullptr != HealthComponent && 0.0f >= HealthComponent->GetCurrentHealth())
	{
		return;
	}
	
	if (nullptr == CombatComponent)
	{
		return;
	}
	
	const bool bHitReactionStarted = CombatComponent->RequestHitReaction(DamageInfo);
	
	if (true == bHitReactionStarted && nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->HandleCombatActionStarted(EPlayerCombatMovementCleanupMode::StopSprintOnly);
	}
}

void ACombatPlayerCharacter::HandleDeath()
{
	UE_LOG(LogCombatPortfolio, Warning, TEXT("Player died"));
	
	StopMove();
	
	if (nullptr != CombatComponent)
	{
		CombatComponent->RequestDeath();
	}
	
	if (nullptr != LockOnComponent)
	{
		LockOnComponent->ClearLockOnTarget();
	}
	
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	
	if (nullptr != MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
	
	if (nullptr != PlayerLocomotionComponent)
	{
		PlayerLocomotionComponent->HandleCombatActionStarted(EPlayerCombatMovementCleanupMode::StopMovementImmediately);
	}
	
	if (nullptr != PlayerHUDComponent)
	{
		PlayerHUDComponent->ShowDeathMessage();
	}
}

void ACombatPlayerCharacter::HandleLockOnTargetChanged()
{
	if (nullptr != PlayerLocomotionComponent)
	{
		if (true == PlayerLocomotionComponent->IsLockedOn())
		{
			PlayerLocomotionComponent->SetRotationMode(ECombatRotationMode::Strafe);
		}
		else
		{
			PlayerLocomotionComponent->SetRotationMode(ECombatRotationMode::OrientToMovement);
		}
	
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
		return TEXT("HitReacting");
	case ECombatActionState::Parrying:
		return TEXT("Parrying");
	case ECombatActionState::Dead:
		return TEXT("Dead");
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
	
	const FCombatAttackEntry* CurrentAttackEntry = CombatComponent->GetCurrentAttackEntry();
	
	const float Damage = nullptr != CurrentAttackEntry ? CurrentAttackEntry->Damage : 0.0f;
	const float Knockback = nullptr != CurrentAttackEntry ? CurrentAttackEntry->KnockbackStrength : 0.0f;
	const FString Strength = nullptr != CurrentAttackEntry ? GetCombatHitStrengthDebugString(CurrentAttackEntry->HitStrength) : TEXT("None");
	
	return FString::Printf(TEXT("%d | Damage: %.1f | Knockback: %.1f | Strength: %s | Window: %s | Buffer: %s"), 
		DisplayComboIndex, 
		Damage,
		Knockback,
		*Strength, 
		*WindowState, 
		*BufferedState);
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
	
	FString MovementStateString = nullptr != PlayerLocomotionComponent ? PlayerLocomotionComponent->GetMovementStateDebugString() : TEXT("None");
	
	FString RotationModeString = nullptr != PlayerLocomotionComponent ? PlayerLocomotionComponent->GetRotationModeDebugString() : TEXT("None");
	
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

