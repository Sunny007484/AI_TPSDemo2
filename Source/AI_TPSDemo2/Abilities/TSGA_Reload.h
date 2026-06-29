// 换弹能力：Montage 等待 + 弹药结算（模块5）。

#pragma once

#include "CoreMinimal.h"
#include "Core/TSGameplayAbility.h"
#include "TSGA_Reload.generated.h"

UCLASS()
class AI_TPSDEMO2_API UTSGA_Reload : public UTSGameplayAbility
{
	GENERATED_BODY()

public:
	UTSGA_Reload();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY()
	FTimerHandle ReloadTimerHandle;

	void FinishReload();
	bool CanReloadWeapon() const;

	UFUNCTION()
	void OnReloadTimerElapsed();
};
