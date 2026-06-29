// Base GameplayAbility for the TPS demo (Module 1): carries an InputTag and activation policy.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "TSGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class ETSActivationPolicy : uint8
{
	// Activate once when the bound input is pressed.
	OnInputTriggered,
	// Stay active while the bound input is held; ends on release.
	WhileInputActive,
	// Activate automatically when granted.
	OnSpawn
};

UCLASS(Abstract)
class AI_TPSDEMO2_API UTSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// Input tag used by UTSAbilitySystemComponent to map presses/releases to this ability.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Input")
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Activation")
	ETSActivationPolicy ActivationPolicy = ETSActivationPolicy::OnInputTriggered;

	ETSActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

protected:
	// Convenience accessors usable by derived abilities.
	UFUNCTION(BlueprintPure, Category = "TS|Ability")
	class ACharacter* GetCharacterFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "TS|Ability")
	class ATSPlayerCharacter* GetTSPlayerCharacterFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "TS|Ability")
	class UTSCombatComponent* GetCombatComponentFromActorInfo() const;
};
