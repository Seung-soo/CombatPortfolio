// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatEnemyBase.h"

#include "CombatPortfolio/CombatPortfolio.h"
#include "CombatPortfolio/AI/CombatEnemyAIController.h"
#include "CombatPortfolio/Components/EnemyAttackComponent.h"
#include "CombatPortfolio/Components/EnemyHealthBarComponent.h"
#include "CombatPortfolio/Components/HealthComponent.h"
#include "CombatPortfolio/Components/HitStopComponent.h"
#include "CombatPortfolio/Components/LockOnMarkerComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACombatEnemyBase::ACombatEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AIControllerClass = ACombatEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;


	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	BodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComponent"));
	BodyMeshComponent->SetupAttachment(GetCapsuleComponent());
	BodyMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
	BodyMeshComponent->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	EnemyAttackComponent = CreateDefaultSubobject<UEnemyAttackComponent>(TEXT("EnemyAttackComponent"));
	
	LockOnMarkerComponent = CreateDefaultSubobject<ULockOnMarkerComponent>(TEXT("LockOnMarkerComponent"));
	LockOnMarkerComponent->SetupAttachment(GetCapsuleComponent());
	
	EnemyHealthBarComponent = CreateDefaultSubobject<UEnemyHealthBarComponent>(TEXT("EnemyHealthBarComponent"));
	EnemyHealthBarComponent->SetupAttachment(GetCapsuleComponent());
	
	HitStopComponent = CreateDefaultSubobject<UHitStopComponent>(TEXT("HitStopComponent"));
	
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	
	if (nullptr != MovementComponent)
	{
		MovementComponent->MaxWalkSpeed = 280.0f;
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
		MovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	}
	
	bUseControllerRotationYaw = false;
}

// Called when the game starts or when spawned
void ACombatEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (nullptr != HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ACombatEnemyBase::HandleHealthChanged);
		HealthComponent->OnDeath.AddDynamic(this, &ACombatEnemyBase::HandleDeath);
		
		if (nullptr != EnemyHealthBarComponent)
		{
			EnemyHealthBarComponent->InitializeHealth(HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
		}
	}
}

void ACombatEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	
	if (nullptr != World)
	{
		World->GetTimerManager().ClearTimer(CriticalAttackWindowTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ACombatEnemyBase::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
	if (nullptr != EnemyHealthBarComponent)
	{
		EnemyHealthBarComponent->SetHealth(CurrentHealth, MaxHealth);
	}
	
	UE_LOG(LogTemp, Log, TEXT("%s Health Changed: %.1f / %.1f, Delta: %.1f"),*GetName(), CurrentHealth, MaxHealth, Delta);
}

void ACombatEnemyBase::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("died: %s"), *GetName());
	
	ApplyDeathState();
}

void ACombatEnemyBase::ApplyDeathState()
{
	CloseCriticalAttackWindow();
	
	UCapsuleComponent* EnemyCapsuleComponent = GetCapsuleComponent();
	
	if (nullptr != EnemyCapsuleComponent)
	{
		EnemyCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	
	if (nullptr != MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	if (nullptr != EnemyAttackComponent)
	{
		EnemyAttackComponent->StopAutoAttack();
	}
	
	if (nullptr != LockOnMarkerComponent)
	{
		LockOnMarkerComponent->HideMarker();
	}
	
	if (nullptr != EnemyHealthBarComponent)
	{
		EnemyHealthBarComponent->HideHealthBar();
	}
	
	AAIController* EnemyAIController = Cast<AAIController>(GetController());
	
	if (nullptr != EnemyAIController)
	{
		EnemyAIController->StopMovement();
	}
}

bool ACombatEnemyBase::RequestParriedReaction()
{
	return false;
}

void ACombatEnemyBase::OpenCriticalAttackWindow()
{
	if (nullptr != HealthComponent && true == HealthComponent->IsDead())
	{
		return;
	}
	
	bCriticalAttackAvailable = true;
	
	UWorld* World = GetWorld();
	
	if (nullptr != World)
	{
		World->GetTimerManager().ClearTimer(CriticalAttackWindowTimerHandle);
		
		World->GetTimerManager().SetTimer(
			CriticalAttackWindowTimerHandle,
			this,
			&ACombatEnemyBase::CloseCriticalAttackWindow,
			CriticalAttackWindowDuration,
			false
		);
	}
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("%s Critical window opened. Duration: %.2f"), *GetName(), CriticalAttackWindowDuration);
}

UHealthComponent* ACombatEnemyBase::GetHealthComponent() const
{
	return HealthComponent.Get();
}

UEnemyAttackComponent* ACombatEnemyBase::GetEnemyAttackComponent() const
{
	return EnemyAttackComponent.Get();
}

ULockOnMarkerComponent* ACombatEnemyBase::GetLockOnMarkerComponent() const
{
	return LockOnMarkerComponent.Get();
}

UEnemyHealthBarComponent* ACombatEnemyBase::GetEnemyHealthBarComponent() const
{
	return EnemyHealthBarComponent.Get();
}

bool ACombatEnemyBase::ConsumeCriticalAttackOpportunity(AActor* AttackerActor)
{
	if (false == bCriticalAttackAvailable)
	{
		return false;
	}
	
	CloseCriticalAttackWindow();
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("%s Critical opportunity consumed by %s"),
		*GetName(), *GetNameSafe(AttackerActor));
	
	return true;
}

bool ACombatEnemyBase::IsCriticalAttackAvailable() const
{
	return bCriticalAttackAvailable;
}

void ACombatEnemyBase::CloseCriticalAttackWindow()
{
	if (false == bCriticalAttackAvailable)
	{
		return;
	}
	
	bCriticalAttackAvailable = false;
	
	UWorld* World = GetWorld();
	
	if (nullptr != World)
	{
		World->GetTimerManager().ClearTimer(CriticalAttackWindowTimerHandle);
	}
	
	UE_LOG(LogCombatPortfolio, Log, TEXT("%s Critical window closed."), *GetName());
}
