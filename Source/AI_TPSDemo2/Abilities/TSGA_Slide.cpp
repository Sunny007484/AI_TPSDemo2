#include "Abilities/TSGA_Slide.h"

#include "Abilities/TSAbilityCommon.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Character/TSCharacterMovementComponent.h"
#include "Core/TSGameplayTags.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UTSGA_Slide::UTSGA_Slide()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ETSActivationPolicy::OnInputTriggered;
	InputTag = TSGameplayTags::Ability_Slide;

	ActivationRequiredTags.AddTag(TSGameplayTags::State_Movement_Sprinting);
	ActivationBlockedTags.AddTag(TSGameplayTags::Cooldown_Slide);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Movement_Sliding);
	ActivationOwnedTags.AddTag(TSGameplayTags::State_Movement_Sliding);
}

void UTSGA_Slide::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UTSCharacterMovementComponent* Movement = TSGetMovementFromActorInfo(ActorInfo);
	if (!Movement || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Movement->StartSlide();
	SlideMovementEndedHandle = Movement->OnSlideMovementEnded.AddUObject(this, &UTSGA_Slide::OnSlideMovementEnded);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.f, FColor::Green, TEXT("Slide: 状态激活成功"));
	}
	UE_LOG(LogTemp, Log, TEXT("Slide: 状态激活成功"));

	// 冷却：给 ASC 添加 Cooldown.Slide tag，CooldownDuration 后移除。
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(TSGameplayTags::Cooldown_Slide);

		if (UWorld* World = GetWorld())
		{
			TWeakObjectPtr<UAbilitySystemComponent> WeakASC(ASC);
			World->GetTimerManager().SetTimer(CooldownTimerHandle, FTimerDelegate::CreateWeakLambda(ASC,
				[WeakASC]()
				{
					if (WeakASC.IsValid())
					{
						WeakASC->RemoveLooseGameplayTag(TSGameplayTags::Cooldown_Slide);
					}
				}), CooldownDuration, false);
		}
	}

	// 计时结束滑铲。
	if (UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, Movement->GetSlideDuration()))
	{
		WaitTask->OnFinish.AddDynamic(this, &UTSGA_Slide::OnSlideTimerFinished);
		WaitTask->ReadyForActivation();
	}
}

void UTSGA_Slide::OnSlideTimerFinished()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UTSGA_Slide::OnSlideMovementEnded()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UTSGA_Slide::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UTSCharacterMovementComponent* Movement = TSGetMovementFromActorInfo(ActorInfo))
	{
		Movement->OnSlideMovementEnded.Remove(SlideMovementEndedHandle);
		SlideMovementEndedHandle.Reset();
		Movement->EndSlide();
	}
	if (ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr)
	{
		// 滑铲默认结束起身（Slide→Walk）。
		Character->UnCrouch();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
