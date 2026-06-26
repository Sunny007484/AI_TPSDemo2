// Reserved weapon ammo attributes (Module 1). DEMO keeps live ammo on the weapon instance;
// this set is wired for a future GAS-driven ammo path.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Core/TSAttributeSet.h"
#include "TSWeaponAttributeSet.generated.h"

UCLASS()
class AI_TPSDEMO2_API UTSWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UTSWeaponAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AmmoInClip, Category = "Weapon")
	FGameplayAttributeData AmmoInClip;
	ATTRIBUTE_ACCESSORS(UTSWeaponAttributeSet, AmmoInClip);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxClipSize, Category = "Weapon")
	FGameplayAttributeData MaxClipSize;
	ATTRIBUTE_ACCESSORS(UTSWeaponAttributeSet, MaxClipSize);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReserveAmmo, Category = "Weapon")
	FGameplayAttributeData ReserveAmmo;
	ATTRIBUTE_ACCESSORS(UTSWeaponAttributeSet, ReserveAmmo);

protected:
	UFUNCTION()
	void OnRep_AmmoInClip(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxClipSize(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ReserveAmmo(const FGameplayAttributeData& OldValue);
};
