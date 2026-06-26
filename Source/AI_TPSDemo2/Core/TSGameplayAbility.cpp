#include "Core/TSGameplayAbility.h"

#include "GameFramework/Character.h"

ACharacter* UTSGameplayAbility::GetCharacterFromActorInfo() const
{
	return Cast<ACharacter>(GetAvatarActorFromActorInfo());
}
