// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAttackComponent.h"

#include "CombatComponent.h"
#include "HealthComponent.h"
#include "Animation/AnimMontage.h"
#include "CombatPortfolio/Combat/CombatDamageLibrary.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UEnemyAttackComponent::UEnemyAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


// Called when the game starts
void UEnemyAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (true == bStartAttackOnBeginPlay)
	{
		StartAutoAttack();
	}
}

void UEnemyAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAutoAttack();

	Super::EndPlay(EndPlayReason);
}

void UEnemyAttackComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (false == bHitWindowOpen)
	{
		return;
	}
	
	PerformAttackTrace();
}

void UEnemyAttackComponent::StartAutoAttack()
{
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(AutoAttackTimerHandle);
	
	World->GetTimerManager().SetTimer(
		AutoAttackTimerHandle,
		this,
		&UEnemyAttackComponent::AttackOnce,
		AttackInterval,
		true
	);
}

void UEnemyAttackComponent::StopAutoAttack()
{
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	World->GetTimerManager().ClearTimer(AutoAttackTimerHandle);
}

void UEnemyAttackComponent::AttackOnce()
{
	ResetHitActors();
	
	OpenAttackHitWindow();
	PerformAttackTrace();
	CloseAttackHitWindow();
	
	EndAttack();
}

bool UEnemyAttackComponent::RequestAttack()
{
	if (false == CanRequestAttack())
	{
		return false;
	}
	
	ResetHitActors();
	
	const bool bMontageStarted = TryPlayAttackMontage();
	
	if (true == bMontageStarted)
	{
		bAttacking = true;
		BeginAttackCooldown();
		return true;
	}
	
	AttackOnce();
	BeginAttackCooldown();
	
	return true;
}

bool UEnemyAttackComponent::CanRequestAttack() const
{
	return false == bAttackOnCooldown && false == bAttacking;
}

bool UEnemyAttackComponent::IsAttackOnCooldown() const
{
	return bAttackOnCooldown;
}

bool UEnemyAttackComponent::IsAttacking() const
{
	return bAttacking;
}

bool UEnemyAttackComponent::IsHitWindowOpen() const
{
	return bHitWindowOpen;
}

void UEnemyAttackComponent::SetStartAttackOnBeginPlay(bool bNewStartAttackOnBeginPlay)
{
	bStartAttackOnBeginPlay = bNewStartAttackOnBeginPlay;
}

void UEnemyAttackComponent::OpenAttackHitWindow()
{
	if (false == bAttacking)
	{
		bAttacking = true;
	}
	
	bHitWindowOpen = true;
	SetComponentTickByHitWindow();
	
	UE_LOG(LogTemp, Log, TEXT("Enemy Attack hit window opened."));
}

void UEnemyAttackComponent::CloseAttackHitWindow()
{
	bHitWindowOpen = false;
	SetComponentTickByHitWindow();
	
	UE_LOG(LogTemp, Log, TEXT("Enemy attack hit window closed."));
}

void UEnemyAttackComponent::EndAttack()
{
	if (false == bAttacking && false == bHitWindowOpen)
	{
		return;
	}
	
	CloseAttackHitWindow();
	
	bAttacking = false;
	ResetHitActors();

	UE_LOG(LogTemp, Log, TEXT("Enemy attack ended."));
}

void UEnemyAttackComponent::CancelAttack()
{
	if (false == bAttacking && false == bHitWindowOpen)
	{
		return;
	}
	
	CloseAttackHitWindow();
	
	bAttacking = false;
	ResetHitActors();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr != OwnerCharacter && nullptr != AttackMontage)
	{
		OwnerCharacter->StopAnimMontage(AttackMontage);
	}
}

bool UEnemyAttackComponent::TryPlayAttackMontage()
{
	if (nullptr == AttackMontage)
	{
		return false;
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (nullptr == OwnerCharacter)
	{
		return false;
	}
	
	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	
	if (nullptr == OwnerMesh)
	{
		return false;
	}
	
	UAnimInstance* AnimInstance = OwnerMesh->GetAnimInstance();
	
	if (nullptr == AnimInstance)
	{
		return false;
	}
	
	const float MontageLength = AnimInstance->Montage_Play(AttackMontage, AttackMontagePlayRate);
	
	if (0.0f >= MontageLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to play enemy attack montage."), *OwnerCharacter->GetName());
		return false;
	}
	
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UEnemyAttackComponent::HandleAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AttackMontage);
	
	UE_LOG(LogTemp, Log, TEXT("%s played enemy attack montage. Length: %.2f"), *OwnerCharacter->GetName(), MontageLength);
	
	return true;
}

void UEnemyAttackComponent::PerformAttackTrace()
{
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return;
	}
	
	const FVector StartLocation = GetAttackTraceStartLocation();
	const FVector EndLocation = GetAttackTraceEndLocation();
	
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	
	const bool bHit = World->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackTraceRadius),
		QueryParams
	);
	
	if (true == bDrawAttackDebug)
	{
		const FColor DebugColor = true == bHit ? FColor::Red : FColor::Blue;
		
		const FVector Center = (StartLocation + EndLocation) * 0.5f;
		const float CapsuleHalfHeight = (EndLocation - StartLocation).Size() * 0.5f + AttackTraceRadius;
		
		DrawDebugCapsule(
			World,
			Center,
			CapsuleHalfHeight,
			AttackTraceRadius,
			FQuat::Identity,
			DebugColor,
			false,
			0.08f
		);
	}
	
	if (false == bHit)
	{
		return;
	}
	
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		
		if (nullptr == HitActor)
		{
			continue;
		}
		
		if (HitActor == OwnerActor)
		{
			continue;
		}
		
		if (true == HasAlreadyHitActor(HitActor))
		{
			continue;
		}
		
		const bool bDamageBlocked = IsDamageBlockedByInvincibility(HitActor);
		
		if (true == bDamageBlocked)
		{
			AddHitActor(HitActor);
			
			UE_LOG(LogTemp, Log, TEXT("Enemy damage blocked by invincibility: %s"), *HitActor->GetName());
			
			if (nullptr != GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Enemy Attack Blocked by IFrame"));
			}
			
			continue;
		}
		
		ApplyDamageToActor(HitResult);
		AddHitActor(HitActor);
	}
}

void UEnemyAttackComponent::ApplyDamageToActor(const FHitResult& HitResult)
{
	AActor* TargetActor = HitResult.GetActor();
	
	if (nullptr == TargetActor)
	{
		return;
	}
	
	if (true == IsDamageBlockedByInvincibility(TargetActor))
	{
		return;
	}
	
	UHealthComponent* HealthComponent = TargetActor->FindComponentByClass<UHealthComponent>();
	
	if (nullptr == HealthComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy hit actor has no HealthComponent: %s"), *TargetActor->GetName());
		return;
	}
	
	if (true == HealthComponent->IsDead())
	{
		return;
	}
	
	AActor* OwnerActor = GetOwner();
	
	FCombatDamageInfo DamageInfo;
	DamageInfo.DamageAmount = AttackDamage;
	DamageInfo.InstigatorActor = OwnerActor;
	DamageInfo.DamageCauser = OwnerActor;
	DamageInfo.HitActor = TargetActor;
	DamageInfo.HitLocation = HitResult.ImpactPoint;
	DamageInfo.HitNormal = HitResult.ImpactNormal;
	DamageInfo.HitDirection = nullptr != OwnerActor ? 
		(TargetActor->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal() : FVector::ZeroVector;
	DamageInfo.HitStrength = HitStrength;
	DamageInfo.HitDirectionType = UCombatDamageLibrary::CalculateHitDirectionFromIncomingDirection(TargetActor, DamageInfo.HitDirection);
	DamageInfo.KnockbackStrength = KnockbackStrength;
	DamageInfo.HitStopDuration = HitStopDuration;
	DamageInfo.HitStopTimeDilation = HitStopTimeDilation;
	
	const bool bDamageApplied = HealthComponent->ApplyDamage(DamageInfo);
	
	if (false == bDamageApplied)
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy damage was not applied to: %s"), *TargetActor->GetName());
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Enemy applied %.1f damage to %s. HP: %.1f / %.1f"), DamageInfo.DamageAmount, *TargetActor->GetName(), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
}

bool UEnemyAttackComponent::IsDamageBlockedByInvincibility(AActor* TargetActor) const
{
	if (nullptr == TargetActor)
	{
		return false;
	}
	
	const UCombatComponent* TargetCombatComponent = TargetActor->FindComponentByClass<UCombatComponent>();
	
	if (nullptr == TargetCombatComponent)
	{
		return false;
	}
	
	return TargetCombatComponent->IsInvincible();
}

bool UEnemyAttackComponent::HasAlreadyHitActor(const AActor* TargetActor) const
{
	if (nullptr == TargetActor)
	{
		return false;
	}
	
	for (const TWeakObjectPtr<AActor>& HitActor : HitActorsThisAttack)
	{
		if (HitActor.Get() == TargetActor)
		{
			return true;
		}
	}
	
	return false;
}

void UEnemyAttackComponent::AddHitActor(AActor* TargetActor)
{
	if (nullptr == TargetActor)
	{
		return;
	}
	
	HitActorsThisAttack.Add(TargetActor);
}

void UEnemyAttackComponent::ResetHitActors()
{
	HitActorsThisAttack.Reset();
}

FVector UEnemyAttackComponent::GetAttackTraceStartLocation() const
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return FVector::ZeroVector;
	}
	
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector ForwardVector = OwnerActor->GetActorForwardVector();
	
	return OwnerLocation + ForwardVector * AttackTraceForwardOffset - FVector(0.0f, 0.0f, AttackTraceHalfHeight);
}

FVector UEnemyAttackComponent::GetAttackTraceEndLocation() const
{
	const AActor* OwnerActor = GetOwner();
	
	if (nullptr == OwnerActor)
	{
		return FVector::ZeroVector;
	}
	
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector ForwardVector = OwnerActor->GetActorForwardVector();
	
	return OwnerLocation + ForwardVector * AttackTraceForwardOffset + FVector(0.0f, 0.0f, AttackTraceHalfHeight);
}

void UEnemyAttackComponent::BeginAttackCooldown()
{
	if (0.0f >= AttackCooldown)
	{
		return;
	}
	
	UWorld* World = GetWorld();
	
	if (nullptr == World)
	{
		return;
	}
	
	bAttackOnCooldown = true;
	
	World->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
	
	World->GetTimerManager().SetTimer(
		AttackCooldownTimerHandle,
		this,
		&UEnemyAttackComponent::EndAttackCooldown,
		AttackCooldown,
		false
	);
}

void UEnemyAttackComponent::EndAttackCooldown()
{
	bAttackOnCooldown = false;
}

void UEnemyAttackComponent::SetComponentTickByHitWindow()
{
	SetComponentTickEnabled(bHitWindowOpen);
}

void UEnemyAttackComponent::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != AttackMontage)
	{
		return;
	}
	
	EndAttack();
}
