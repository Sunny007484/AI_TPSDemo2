// 能力模块共享辅助函数（避免 unity build 下匿名命名空间冲突）。

#pragma once

#include "Character/TSCharacterMovementComponent.h"
#include "GameFramework/Character.h"

inline UTSCharacterMovementComponent* TSGetMovementFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo)
	{
		return nullptr;
	}
	if (const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		return Cast<UTSCharacterMovementComponent>(Character->GetCharacterMovement());
	}
	return nullptr;
}
