#include "Abilities/TSGE_Damage.h"

#include "Core/TSAttributeSet.h"
#include "Core/TSGameplayTags.h"

UTSGE_Damage::UTSGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo ModInfo;
	ModInfo.Attribute = UTSAttributeSet::GetDamageAttribute();
	ModInfo.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = TSGameplayTags::Data_Damage;
	ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(ModInfo);
}
