// Project AbilitySystemComponent (Module 1): startup ability granting + input-tag activation.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "TSAbilitySystemComponent.generated.h"

UCLASS()
class AI_TPSDEMO2_API UTSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Grants the given abilities (authority only). OnSpawn-policy abilities are activated immediately.
	void GrantStartupAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);

	// Called from the character's input handlers to drive ability activation by input tag.
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

private:
	bool bStartupAbilitiesGranted = false;
};
