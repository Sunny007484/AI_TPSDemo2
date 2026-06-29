#include "Core/TSGameplayAbility.h"

#include "Character/TSPlayerCharacter.h"
#include "GameFramework/Character.h"
#include "Weapon/TSCombatComponent.h"

ACharacter* UTSGameplayAbility::GetCharacterFromActorInfo() const
{
	return Cast<ACharacter>(GetAvatarActorFromActorInfo());
}

ATSPlayerCharacter* UTSGameplayAbility::GetTSPlayerCharacterFromActorInfo() const
{
	return Cast<ATSPlayerCharacter>(GetAvatarActorFromActorInfo());
}

UTSCombatComponent* UTSGameplayAbility::GetCombatComponentFromActorInfo() const
{
	if (const ACharacter* Character = GetCharacterFromActorInfo())
	{
		return Character->FindComponentByClass<UTSCombatComponent>();
	}
	return nullptr;
}
