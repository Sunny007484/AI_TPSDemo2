// 射击能力：射线检测 + 伤害 GE + 弹药消耗 + 后坐力（模块5）。

#pragma once

#include "CoreMinimal.h"
#include "Core/TSGameplayAbility.h"
#include "TSGA_Fire.generated.h"

class UAbilityTask_WaitDelay;

UCLASS()
class AI_TPSDEMO2_API UTSGA_Fire : public UTSGameplayAbility
{
	GENERATED_BODY()

public:
	UTSGA_Fire();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "TS|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY()
	int32 ConsecutiveShots = 0;

	UPROPERTY()
	FTimerHandle FireTimerHandle;

	void FireOnce();
	void ScheduleNextShot();
	bool PerformFireTrace(float& OutDamage) const;
	float GetCurrentSpreadDegrees() const;
	void ApplyRecoil();
	void PlayFireFeedback();
	// 弹匣空且仍有备弹时触发换弹能力。
	void TryTriggerAutoReload() const;

	UFUNCTION()
	void OnFireTimerElapsed();
};
