#include "Abilities/TSGA_ADS.h"

#include "Camera/CameraComponent.h"
#include "Character/TSCharacterMovementComponent.h"
#include "Character/TSPlayerCharacter.h"
#include "Core/TSGameplayTags.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"
#include "Weapon/TSCombatComponent.h"
#include "Weapon/TSWeaponBase.h"
#include "Weapon/TSWeaponDataAsset.h"

UTSGA_ADS::UTSGA_ADS()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ETSActivationPolicy::WhileInputActive;
	InputTag = TSGameplayTags::Ability_ADS;

	ActivationOwnedTags.AddTag(TSGameplayTags::State_Combat_ADS);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Dead);
}

void UTSGA_ADS::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UTSCharacterMovementComponent* Movement = Cast<UTSCharacterMovementComponent>(
		GetCharacterFromActorInfo() ? GetCharacterFromActorInfo()->GetCharacterMovement() : nullptr))
	{
		Movement->SetAimingDownSights(true);
	}

	BeginADSTransition(true);
}

void UTSGA_ADS::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTSGA_ADS::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ADSTimerHandle);
	}

	// 松开 ADS 时立即还原相机
	if (ATSPlayerCharacter* PlayerChar = GetTSPlayerCharacterFromActorInfo())
	{
		if (UCameraComponent* Camera = PlayerChar->GetFollowCamera())
		{
			Camera->SetFieldOfView(HipFOV > 0.f ? HipFOV : 90.f);
		}
		if (USpringArmComponent* Boom = PlayerChar->GetCameraBoom())
		{
			Boom->SocketOffset = HipSocketOffset.IsNearlyZero()
				? FVector(0.f, 50.f, 60.f) : HipSocketOffset;
		}
	}

	if (UTSCharacterMovementComponent* Movement = Cast<UTSCharacterMovementComponent>(
		GetCharacterFromActorInfo() ? GetCharacterFromActorInfo()->GetCharacterMovement() : nullptr))
	{
		Movement->SetAimingDownSights(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTSGA_ADS::BeginADSTransition(bool bToADS)
{
	const ATSPlayerCharacter* PlayerChar = GetTSPlayerCharacterFromActorInfo();
	if (!PlayerChar)
	{
		return;
	}

	if (const USpringArmComponent* Boom = PlayerChar->GetCameraBoom())
	{
		HipSocketOffset = Boom->SocketOffset;
	}
	if (const UCameraComponent* Camera = PlayerChar->GetFollowCamera())
	{
		HipFOV = Camera->FieldOfView;
	}

	float ADSFOV = 55.f;
	FVector ADSOffset = FVector(0.f, 30.f, 40.f);
	if (UTSCombatComponent* Combat = GetCombatComponentFromActorInfo())
	{
		if (const ATSWeaponBase* Weapon = Combat->GetCurrentWeapon())
		{
			if (const UTSWeaponDataAsset* Data = Weapon->GetWeaponData())
			{
				ADSFOV = Data->ADS_FOV;
				ADSOffset = Data->ADS_CameraOffset;
			}
		}
	}

	bTransitioningToADS = bToADS;
	TargetFOV = bToADS ? ADSFOV : HipFOV;
	TargetSocketOffset = bToADS ? ADSOffset : HipSocketOffset;
	TransitionElapsed = 0.f;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ADSTimerHandle, this, &UTSGA_ADS::OnADSTimerTick,
			0.016f, true);
	}
}

void UTSGA_ADS::OnADSTimerTick()
{
	UpdateADSTransition();
}

void UTSGA_ADS::UpdateADSTransition()
{
	TransitionElapsed += 0.016f;
	const float Alpha = FMath::Clamp(TransitionElapsed / ADSTransitionTime, 0.f, 1.f);
	ApplyCameraState(Alpha);

	if (Alpha >= 1.f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ADSTimerHandle);
		}
	}
}

void UTSGA_ADS::ApplyCameraState(float Alpha) const
{
	const ATSPlayerCharacter* PlayerChar = GetTSPlayerCharacterFromActorInfo();
	if (!PlayerChar)
	{
		return;
	}

	const float StartFOV = bTransitioningToADS ? HipFOV : TargetFOV;
	const float EndFOV = bTransitioningToADS ? TargetFOV : HipFOV;
	const FVector StartOffset = bTransitioningToADS ? HipSocketOffset : TargetSocketOffset;
	const FVector EndOffset = bTransitioningToADS ? TargetSocketOffset : HipSocketOffset;

	if (UCameraComponent* Camera = PlayerChar->GetFollowCamera())
	{
		Camera->SetFieldOfView(FMath::Lerp(StartFOV, EndFOV, Alpha));
	}

	if (USpringArmComponent* Boom = PlayerChar->GetCameraBoom())
	{
		Boom->SocketOffset = FMath::Lerp(StartOffset, EndOffset, Alpha);
	}
}
