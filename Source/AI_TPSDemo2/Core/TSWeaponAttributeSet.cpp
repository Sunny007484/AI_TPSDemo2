#include "Core/TSWeaponAttributeSet.h"

#include "Net/UnrealNetwork.h"

UTSWeaponAttributeSet::UTSWeaponAttributeSet()
{
	InitAmmoInClip(30.f);
	InitMaxClipSize(30.f);
	InitReserveAmmo(90.f);
}

void UTSWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTSWeaponAttributeSet, AmmoInClip, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTSWeaponAttributeSet, MaxClipSize, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTSWeaponAttributeSet, ReserveAmmo, COND_None, REPNOTIFY_Always);
}

void UTSWeaponAttributeSet::OnRep_AmmoInClip(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTSWeaponAttributeSet, AmmoInClip, OldValue);
}

void UTSWeaponAttributeSet::OnRep_MaxClipSize(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTSWeaponAttributeSet, MaxClipSize, OldValue);
}

void UTSWeaponAttributeSet::OnRep_ReserveAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTSWeaponAttributeSet, ReserveAmmo, OldValue);
}
