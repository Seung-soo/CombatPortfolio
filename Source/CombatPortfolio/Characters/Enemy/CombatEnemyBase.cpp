// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatEnemyBase.h"
#include "CombatPortfolio/Components/EnemyAttackComponent.h"
#include "CombatPortfolio/Components/EnemyHealthBarComponent.h"
#include "CombatPortfolio/Components/HealthComponent.h"
#include "CombatPortfolio/Components/LockOnMarkerComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ACombatEnemyBase::ACombatEnemyBase()
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
	if (nullptr != CapsuleComponent)
	{
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
