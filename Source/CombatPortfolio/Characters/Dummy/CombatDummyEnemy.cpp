// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatDummyEnemy.h"

#include "CombatPortfolio/Components/EnemyAttackComponent.h"
#include "CombatPortfolio/Components/EnemyHealthBarComponent.h"
#include "CombatPortfolio/Components/HealthComponent.h"
#include "CombatPortfolio/Components/LockOnMarkerComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ACombatDummyEnemy::ACombatDummyEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);
	
	CapsuleComponent->InitCapsuleSize(42.0f, 96.0f);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CapsuleComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
	MeshComponent->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	EnemyAttackComponent = CreateDefaultSubobject<UEnemyAttackComponent>(TEXT("EnemyAttackComponent"));
	
	LockOnMarkerComponent = CreateDefaultSubobject<ULockOnMarkerComponent>(TEXT("LockOnMarkerComponent"));
	LockOnMarkerComponent->SetupAttachment(CapsuleComponent);
	
	EnemyHealthBarComponent = CreateDefaultSubobject<UEnemyHealthBarComponent>(TEXT("EnemyHealthBarComponent"));
	EnemyHealthBarComponent->SetupAttachment(CapsuleComponent);
}

// Called when the game starts or when spawned
void ACombatDummyEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (nullptr != HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ACombatDummyEnemy::HandleHealthChanged);
		HealthComponent->OnDeath.AddDynamic(this, &ACombatDummyEnemy::HandleDeath);
		
		if (nullptr != EnemyHealthBarComponent)
		{
			EnemyHealthBarComponent->InitializeHealth(HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
		}
	}
}

void ACombatDummyEnemy::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
	if (nullptr != EnemyHealthBarComponent)
	{
		EnemyHealthBarComponent->SetHealth(CurrentHealth, MaxHealth);
	}
	
	UE_LOG(LogTemp, Log, TEXT("DummyEnemy Health Changed: %.1f / %.1f, Delta: %.1f"), CurrentHealth, MaxHealth, Delta);
}

void ACombatDummyEnemy::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("DummyEnemy died: %s"), *GetName());
	
	ApplyDeathState();
}

void ACombatDummyEnemy::ApplyDeathState()
{
	if (nullptr != CapsuleComponent)
	{
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	if (nullptr != MeshComponent)
	{
		MeshComponent->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.35f));
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -85.0f));
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
}
