// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatDamageLibrary.h"

#include "NiagaraFunctionLibrary.h"
#include "CombatPortfolio/Components/HitStopComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ECombatHitDirection UCombatDamageLibrary::CalculateHitDirectionFromActors(const AActor* HitActor,
                                                                          const AActor* InstigatorActor)
{
	if (nullptr == HitActor || nullptr == InstigatorActor)
	{
		return ECombatHitDirection::Front;
	}
	
	const FVector DirectionToInstigator = (InstigatorActor->GetActorLocation() - HitActor->GetActorLocation()).GetSafeNormal2D();
	
	return CalculateHitDirectionFromIncomingDirection(HitActor, -DirectionToInstigator);
}

ECombatHitDirection UCombatDamageLibrary::CalculateHitDirectionFromIncomingDirection(const AActor* HitActor,
	const FVector& IncomingDirection)
{
	if (nullptr == HitActor)
	{
		return ECombatHitDirection::Front;
	}
	
	const FVector IncomingDirection2D = IncomingDirection.GetSafeNormal2D();
	
	if (true == IncomingDirection2D.IsNearlyZero())
	{
		return ECombatHitDirection::Front;
	}
	
	const FVector DirectionToInstigator = -IncomingDirection2D;
	
	const FVector HitActorForward = HitActor->GetActorForwardVector().GetSafeNormal2D();
	const FVector HitActorRight = HitActor->GetActorRightVector().GetSafeNormal2D();
	
	const float ForwardDot = FVector::DotProduct(HitActorForward, DirectionToInstigator);
	const float RightDot = FVector::DotProduct(HitActorRight, DirectionToInstigator);
	
	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		return ForwardDot >= 0.0f ? ECombatHitDirection::Front : ECombatHitDirection::Back;
	}
	
	return RightDot >= 0.0f ? ECombatHitDirection::Right : ECombatHitDirection::Left;
}

bool UCombatDamageLibrary::ApplyKnockbackFromDamageInfo(const FCombatDamageInfo& DamageInfo)
{
	if (0.0f >= DamageInfo.KnockbackStrength)
	{
		return false;
	}
	
	ACharacter* HitCharacter = Cast<ACharacter>(DamageInfo.HitActor);
	
	if (nullptr == HitCharacter)
	{
		return false;
	}
	
	FVector KnockbackDirection = DamageInfo.HitDirection.GetSafeNormal2D();
	
	if (true == KnockbackDirection.IsNearlyZero())
	{
		if (nullptr != DamageInfo.InstigatorActor)
		{
			KnockbackDirection = (HitCharacter->GetActorLocation() - DamageInfo.InstigatorActor->GetActorLocation()).GetSafeNormal2D();
		}
	}
	
	if (true == KnockbackDirection.IsNearlyZero())
	{
		return false;
	}
	
	const FVector LaunchVelocity = KnockbackDirection * DamageInfo.KnockbackStrength;
	
	HitCharacter->LaunchCharacter(LaunchVelocity, true, false);
	
	return true;
}

void UCombatDamageLibrary::ApplyHitStopFromDamageInfo(const FCombatDamageInfo& DamageInfo)
{
	if (0.0f >= DamageInfo.HitStopDuration)
	{
		return;
	}	
	
	const float TimeDilation = FMath::Clamp(DamageInfo.HitStopTimeDilation, 0.01f, 1.0f);
	
	if (nullptr != DamageInfo.InstigatorActor)
	{
		UHitStopComponent* InstigatorHitStopComponent = DamageInfo.InstigatorActor->FindComponentByClass<UHitStopComponent>();
		
		if (nullptr != InstigatorHitStopComponent)
		{
			InstigatorHitStopComponent->RequestHitStop(DamageInfo.HitStopDuration, TimeDilation);
		}
	}
	
	if (nullptr != DamageInfo.HitActor && DamageInfo.HitActor != DamageInfo.InstigatorActor)
	{
		UHitStopComponent* HitActorHitStopComponent = DamageInfo.HitActor->FindComponentByClass<UHitStopComponent>();
		
		if (nullptr != HitActorHitStopComponent)
		{
			HitActorHitStopComponent->RequestHitStop(DamageInfo.HitStopDuration, TimeDilation);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("HitStop Applied | Instigator: %s | Target: %s | Duration: %.3f | TimeDilation: %.2f"),
		*GetNameSafe(DamageInfo.InstigatorActor),
		*GetNameSafe(DamageInfo.HitActor),
		DamageInfo.HitStopDuration,
		TimeDilation);
}

bool UCombatDamageLibrary::PlayCameraShakeFromDamageInfo(const FCombatDamageInfo& DamageInfo)
{
	if (nullptr == DamageInfo.CameraShakeClass)
	{
		return false;
	}
	
	if (0.0f >= DamageInfo.CameraShakeScale)
	{
		return false;
	}
	
	APlayerController* PlayerController = nullptr;
	
	APawn* InstigatorPawn = Cast<APawn>(DamageInfo.InstigatorActor);
	
	if (nullptr != InstigatorPawn)
	{
		PlayerController = Cast<APlayerController>(InstigatorPawn->GetController());
	}
	
	if (nullptr == PlayerController)
	{
		APawn* HitPawn = Cast<APawn>(DamageInfo.HitActor);
		
		if (nullptr != HitPawn)
		{
			PlayerController = Cast<APlayerController>(HitPawn->GetController());
		}
	}
	
	if (nullptr == PlayerController)
	{
		return false;
	}
	
	PlayerController->ClientStartCameraShake(DamageInfo.CameraShakeClass, DamageInfo.CameraShakeScale);
	
	UE_LOG(LogTemp, Log, TEXT("CameraShake played | Controller: %s | Scale: %.2f"), *GetNameSafe(PlayerController), DamageInfo.CameraShakeScale);
	
	return true;
}

void UCombatDamageLibrary::PlayHitFeedbackFromDamageInfo(const FCombatDamageInfo& DamageInfo)
{
	UWorld* World = nullptr;
	
	if (nullptr != DamageInfo.HitActor)
	{
		World = DamageInfo.HitActor->GetWorld();
	}
	
	if (nullptr == World && nullptr != DamageInfo.InstigatorActor)
	{
		World = DamageInfo.InstigatorActor->GetWorld();
	}
	
	if (nullptr == World)
	{
		return;
	}
	
	const FVector SpawnLocation = DamageInfo.HitLocation;
	const FVector HitNormal = DamageInfo.HitNormal.GetSafeNormal();
	
	const FRotator SpawnRotation = false == HitNormal.IsNearlyZero() ? HitNormal.Rotation() : FRotator::ZeroRotator;
	
	if (nullptr != DamageInfo.HitVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			DamageInfo.HitVFX,
			SpawnLocation,
			SpawnRotation,
			FVector(DamageInfo.HitVFXScale)
		);
	}
	
	if (nullptr != DamageInfo.HitSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,
			DamageInfo.HitSFX,
			SpawnLocation,
			DamageInfo.HitSFXVolumeMultiplier,
			DamageInfo.HitSFXPitchMultiplier
		);
	}
	
	UE_LOG(LogTemp, Log, TEXT("HitFeedback Played | Target: %s | Location: %s | VFX: %s | SFX: %s"),
		*GetNameSafe(DamageInfo.HitActor),
		*SpawnLocation.ToString(),
		*GetNameSafe(DamageInfo.HitVFX),
		*GetNameSafe(DamageInfo.HitSFX)
	);
}

void UCombatDamageLibrary::ApplyDamageFeedbackFromDamageInfo(const FCombatDamageInfo& DamageInfo)
{
	PlayHitFeedbackFromDamageInfo(DamageInfo);
	PlayCameraShakeFromDamageInfo(DamageInfo);
	ApplyHitStopFromDamageInfo(DamageInfo);
	ApplyKnockbackFromDamageInfo(DamageInfo);
}
