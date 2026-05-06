// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatEnemyBase.h"

#include "CombatPortfolio/AI/CombatEnemyAIController.h"
#include "CombatPortfolio/Components/EnemyAttackComponent.h"
#include "CombatPortfolio/Components/EnemyHealthBarComponent.h"
#include "CombatPortfolio/Components/HealthComponent.h"
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
