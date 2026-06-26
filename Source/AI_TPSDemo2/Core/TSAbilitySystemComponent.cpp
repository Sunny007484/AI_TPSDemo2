#include "Core/TSAbilitySystemComponent.h"

#include "Core/TSGameplayAbility.h"

void UTSAbilitySystemComponent::GrantStartupAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	if (bStartupAbilitiesGranted || !IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : Abilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		const FGameplayAbilitySpecHandle Handle = GiveAbility(AbilitySpec);

		if (const UTSGameplayAbility* TSAbility = Cast<UTSGameplayAbility>(AbilitySpec.Ability))
		{
			if (TSAbility->GetActivationPolicy() == ETSActivationPolicy::OnSpawn)
			{
				TryActivateAbility(Handle);
			}
		}
	}

	bStartupAbilitiesGranted = true;
}

void UTSAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		const UTSGameplayAbility* TSAbility = Cast<UTSGameplayAbility>(Spec.Ability);
		if (!TSAbility || !TSAbility->InputTag.MatchesTagExact(InputTag))
		{
			continue;
		}

		Spec.InputPressed = true;
		if (Spec.IsActive())
		{
			AbilitySpecInputPressed(Spec);
			if (const UGameplayAbility* Instance = Spec.GetPrimaryInstance())
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle,
					Instance->GetCurrentActivationInfo().GetActivationPredictionKey());
			}
		}
		else
		{
			TryActivateAbility(Spec.Handle);
		}
	}
}

void UTSAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		const UTSGameplayAbility* TSAbility = Cast<UTSGameplayAbility>(Spec.Ability);
		if (!TSAbility || !TSAbility->InputTag.MatchesTagExact(InputTag) || !Spec.IsActive())
		{
			continue;
		}

		Spec.InputPressed = false;
		AbilitySpecInputReleased(Spec);
		if (const UGameplayAbility* Instance = Spec.GetPrimaryInstance())
		{
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle,
				Instance->GetCurrentActivationInfo().GetActivationPredictionKey());
		}
	}
}
