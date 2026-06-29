#include "Abilities/TSGE_InitAttributes.h"

#include "Core/TSAttributeSet.h"

UTSGE_InitAttributes::UTSGE_InitAttributes()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	{
		FGameplayModifierInfo ModInfo;
		ModInfo.Attribute = UTSAttributeSet::GetHealthAttribute();
		ModInfo.ModifierOp = EGameplayModOp::Override;
		FScalableFloat Magnitude;
		Magnitude.Value = 100.f;
		ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(Magnitude);
		Modifiers.Add(ModInfo);
	}

	{
		FGameplayModifierInfo ModInfo;
		ModInfo.Attribute = UTSAttributeSet::GetMaxHealthAttribute();
		ModInfo.ModifierOp = EGameplayModOp::Override;
		FScalableFloat Magnitude;
		Magnitude.Value = 100.f;
		ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(Magnitude);
		Modifiers.Add(ModInfo);
	}
}
