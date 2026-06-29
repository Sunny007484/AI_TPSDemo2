#include "Abilities/TSGA_Reload.h"

#include "Core/TSGameplayTags.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Weapon/TSCombatComponent.h"
#include "Weapon/TSWeaponBase.h"
#include "Weapon/TSWeaponDataAsset.h"

UTSGA_Reload::UTSGA_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ETSActivationPolicy::OnInputTriggered;
	InputTag = TSGameplayTags::Ability_Reload;

	ActivationOwnedTags.AddTag(TSGameplayTags::State_Combat_Reloading);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Weapon_Switching);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Dead);
}

bool UTSGA_Reload::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return CanReloadWeapon();
}

bool UTSGA_Reload::CanReloadWeapon() const
{
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	if (!Weapon || !Weapon->GetWeaponData())
	{
		return false;
	}

	const UTSWeaponDataAsset* Data = Weapon->GetWeaponData();
	return Weapon->GetCurrentAmmo() < Data->ClipSize && Weapon->GetCurrentReserve() > 0;
}

void UTSGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CanReloadWeapon())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	float ReloadDuration = 2.f;
	if (UTSCombatComponent* Combat = GetCombatComponentFromActorInfo())
	{
		if (const ATSWeaponBase* Weapon = Combat->GetCurrentWeapon())
		{
			if (const UTSWeaponDataAsset* Data = Weapon->GetWeaponData())
			{
				ReloadDuration = Data->ReloadTime;

				if (Data->ReloadMontage)
				{
					if (ACharacter* Character = GetCharacterFromActorInfo())
					{
						if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
						{
							AnimInstance->Montage_Play(Data->ReloadMontage);
						}
					}
				}
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UTSGA_Reload::OnReloadTimerElapsed,
			ReloadDuration, false);
	}
}

void UTSGA_Reload::OnReloadTimerElapsed()
{
	FinishReload();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UTSGA_Reload::FinishReload()
{
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	if (Weapon)
	{
		Weapon->ApplyReload();
	}
}

void UTSGA_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
