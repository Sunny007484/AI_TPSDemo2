#include "Abilities/TSGA_WeaponSwitch.h"

#include "AbilitySystemComponent.h"
#include "Core/TSGameplayTags.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Weapon/TSCombatComponent.h"
#include "Weapon/TSWeaponBase.h"
#include "Weapon/TSWeaponDataAsset.h"

namespace
{
	constexpr int32 SwitchNextSlot = -2;
}

UTSGA_WeaponSwitch::UTSGA_WeaponSwitch()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ETSActivationPolicy::OnInputTriggered;
	InputTag = TSGameplayTags::Ability_WeaponSwitch;

	ActivationOwnedTags.AddTag(TSGameplayTags::State_Weapon_Switching);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Dead);
}

bool UTSGA_WeaponSwitch::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	if (!Combat)
	{
		return false;
	}

	const int32 ResolvedSlot = ResolveTargetSlot();
	if (ResolvedSlot == SwitchNextSlot)
	{
		for (int32 Offset = 1; Offset < UTSCombatComponent::MaxWeaponSlots; ++Offset)
		{
			const int32 NextSlot = (Combat->GetCurrentSlotIndex() + Offset) % UTSCombatComponent::MaxWeaponSlots;
			if (Combat->GetWeaponInSlot(NextSlot) && NextSlot != Combat->GetCurrentSlotIndex())
			{
				return true;
			}
		}
		return false;
	}

	return Combat->GetWeaponInSlot(ResolvedSlot) && ResolvedSlot != Combat->GetCurrentSlotIndex();
}

int32 UTSGA_WeaponSwitch::ResolveTargetSlot() const
{
	if (UTSCombatComponent* Combat = GetCombatComponentFromActorInfo())
	{
		const int32 Pending = Combat->GetPendingSwitchSlot();
		if (Pending != INDEX_NONE)
		{
			return Pending;
		}
	}
	return INDEX_NONE;
}

void UTSGA_WeaponSwitch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	if (!Combat)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TargetSlotIndex = ResolveTargetSlot();
	Combat->ClearPendingSwitchSlot();

	// 切枪打断换弹
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(TSGameplayTags::State_Combat_Reloading);
		ASC->CancelAbilities(&CancelTags);
	}

	float SwitchDelay = 0.3f;
	if (TargetSlotIndex >= 0)
	{
		if (const ATSWeaponBase* TargetWeapon = Combat->GetWeaponInSlot(TargetSlotIndex))
		{
			if (const UTSWeaponDataAsset* Data = TargetWeapon->GetWeaponData())
			{
				if (Data->EquipMontage)
				{
					if (ACharacter* Character = GetCharacterFromActorInfo())
					{
						if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
						{
							SwitchDelay = FMath::Max(SwitchDelay, Data->EquipMontage->GetPlayLength());
							AnimInstance->Montage_Play(Data->EquipMontage);
						}
					}
				}
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(SwitchTimerHandle, this, &UTSGA_WeaponSwitch::OnSwitchTimerElapsed,
			SwitchDelay, false);
	}
}

void UTSGA_WeaponSwitch::OnSwitchTimerElapsed()
{
	PerformSwitch();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UTSGA_WeaponSwitch::PerformSwitch()
{
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	if (!Combat)
	{
		return;
	}

	if (TargetSlotIndex == SwitchNextSlot)
	{
		Combat->SwitchNext();
	}
	else if (TargetSlotIndex >= 0)
	{
		Combat->SwitchToSlot(TargetSlotIndex);
	}
}

void UTSGA_WeaponSwitch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SwitchTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
