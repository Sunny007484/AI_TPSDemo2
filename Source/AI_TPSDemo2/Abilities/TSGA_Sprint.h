// 冲刺能力（WhileInputActive）：激活期间赋予 Sprinting tag 并提升移速；ADS/滑铲时被阻止（模块3）。

#pragma once

#include "CoreMinimal.h"
#include "Core/TSGameplayAbility.h"
#include "TSGA_Sprint.generated.h"

UCLASS()
class AI_TPSDEMO2_API UTSGA_Sprint : public UTSGameplayAbility
{
	GENERATED_BODY()

public:
	UTSGA_Sprint();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
