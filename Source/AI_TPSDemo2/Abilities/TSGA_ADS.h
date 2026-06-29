// 机瞄能力：FOV/相机偏移插值 + 移速压制（模块5）。

#pragma once

#include "CoreMinimal.h"
#include "Core/TSGameplayAbility.h"
#include "TSGA_ADS.generated.h"

UCLASS()
class AI_TPSDEMO2_API UTSGA_ADS : public UTSGameplayAbility
{
	GENERATED_BODY()

public:
	UTSGA_ADS();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "TS|ADS")
	float ADSTransitionTime = 0.15f;

	UPROPERTY()
	float HipFOV = 90.f;

	UPROPERTY()
	FVector HipSocketOffset = FVector(0.f, 50.f, 60.f);

	UPROPERTY()
	float TargetFOV = 90.f;

	UPROPERTY()
	FVector TargetSocketOffset = FVector::ZeroVector;

	UPROPERTY()
	float TransitionElapsed = 0.f;

	UPROPERTY()
	bool bTransitioningToADS = true;

	UPROPERTY()
	FTimerHandle ADSTimerHandle;

	void BeginADSTransition(bool bToADS);
	void UpdateADSTransition();
	void ApplyCameraState(float Alpha) const;

	UFUNCTION()
	void OnADSTimerTick();
};
