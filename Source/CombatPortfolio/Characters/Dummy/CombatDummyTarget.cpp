// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatDummyTarget.h"
#include "CombatPortfolio/Components/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// Sets default values
ACombatDummyTarget::ACombatDummyTarget()
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
}

// Called when the game starts or when spawned
void ACombatDummyTarget::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalMeshScale = nullptr != MeshComponent ? MeshComponent->GetRelativeScale3D() : FVector::OneVector;
	
	if (nullptr != HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ACombatDummyTarget::HandleHealthChanged);
		HealthComponent->OnDeath.AddDynamic(this, &ACombatDummyTarget::HandleDeath);
	}
}

void ACombatDummyTarget::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
	UE_LOG(LogTemp,Log, TEXT("DummyTarget Health Changed: %.1f / %.1f, Delta: %.1f"), CurrentHealth, MaxHealth, Delta);
	
	if (0.0f > Delta)
	{
		PlayHitReaction();
	}
}

void ACombatDummyTarget::HandleDeath()
{
	UE_LOG(LogTemp, Log, TEXT("DummyTarget Death Reaction: %s"), *GetName());
	
	ApplyDeathState();
}

void ACombatDummyTarget::PlayHitReaction()
{
	if (nullptr == MeshComponent)
	{
		return;
	}
	
	MeshComponent->SetRelativeScale3D(OriginalMeshScale * HitReactionScaleMultiplier);
	
	UWorld* World = GetWorld();
	
	if (nullptr != World)
	{
		DrawDebugSphere(
			World,
			GetActorLocation() + FVector(0.0f, 0.0f, 80.0f),
			60.0f,
			16,
			FColor::Yellow,
			false,
			HitReactionDuration
		);
		
		World->GetTimerManager().ClearTimer(HitReactionTimerHandle);
		
		World->GetTimerManager().SetTimer(
			HitReactionTimerHandle,
			this,
			&ACombatDummyTarget::ResetHitReaction,
			HitReactionDuration,
			false
		);
	}
}

void ACombatDummyTarget::ResetHitReaction()
{
	if (nullptr == MeshComponent)
	{
		return;
	}
	
	MeshComponent->SetRelativeScale3D(OriginalMeshScale);
}

void ACombatDummyTarget::ApplyDeathState()
{
	if (nullptr != MeshComponent)
	{
		MeshComponent->SetRelativeScale3D(OriginalMeshScale * FVector(1.0f, 1.0f, 0.35f));
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -85.0f));
	}
	
	if (nullptr != CapsuleComponent && true == bDisableCollisionOnDeath)
	{
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
