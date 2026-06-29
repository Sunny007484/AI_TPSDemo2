// 滑铲能力（OnInputTriggered）：需处于冲刺状态，触发前冲滑行，带 1.5s 冷却（模块3）。

#pragma once

#include "CoreMinimal.h"
#include "Core/TSGameplayAbility.h"
#include "Engine/TimerHandle.h"
#include "TSGA_Slide.generated.h"

UCLASS()
class AI_TPSDEMO2_API UTSGA_Slide : public UTSGameplayAbility
{
	GENERATED_BODY()

public:
	UTSGA_Slide();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnSlideTimerFinished();

	void OnSlideMovementEnded();

	// 滑铲冷却时长（用 Cooldown.Slide loose tag 实现，功能等价于 GE_Cooldown_Slide）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float CooldownDuration = 1.5f;

private:
	FTimerHandle CooldownTimerHandle;
	FDelegateHandle SlideMovementEndedHandle;
};
