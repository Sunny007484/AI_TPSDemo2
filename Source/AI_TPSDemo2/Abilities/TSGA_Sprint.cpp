#include "Abilities/TSGA_Sprint.h"

#include "Abilities/TSAbilityCommon.h"
#include "Character/TSCharacterMovementComponent.h"
#include "Core/TSGameplayTags.h"
#include "GameFramework/Character.h"

UTSGA_Sprint::UTSGA_Sprint()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ETSActivationPolicy::WhileInputActive;
	InputTag = TSGameplayTags::Ability_Sprint;

	ActivationOwnedTags.AddTag(TSGameplayTags::State_Movement_Sprinting);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Combat_ADS);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Movement_Sliding);
}

void UTSGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UTSCharacterMovementComponent* Movement = TSGetMovementFromActorInfo(ActorInfo))
	{
		Movement->SetWantsToSprint(true);
	}
}

void UTSGA_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTSGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UTSCharacterMovementComponent* Movement = TSGetMovementFromActorInfo(ActorInfo))
	{
		Movement->SetWantsToSprint(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
