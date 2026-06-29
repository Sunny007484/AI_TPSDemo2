// 切枪能力：Equip 动画 + CombatComponent 切换（模块5）。

#pragma once

#include "CoreMinimal.h"
#include "Core/TSGameplayAbility.h"
#include "TSGA_WeaponSwitch.generated.h"

UCLASS()
class AI_TPSDEMO2_API UTSGA_WeaponSwitch : public UTSGameplayAbility
{
	GENERATED_BODY()

public:
	UTSGA_WeaponSwitch();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY()
	int32 TargetSlotIndex = INDEX_NONE;

	UPROPERTY()
	FTimerHandle SwitchTimerHandle;

	void PerformSwitch();
	int32 ResolveTargetSlot() const;

	UFUNCTION()
	void OnSwitchTimerElapsed();
};
