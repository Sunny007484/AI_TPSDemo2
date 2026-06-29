// 战斗组件：双武器槽持有/附加/切换（模块4）。

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TSCombatComponent.generated.h"

class ATSWeaponBase;
class UTSWeaponDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AI_TPSDEMO2_API UTSCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTSCombatComponent();

	static constexpr int32 MaxWeaponSlots = 2;

	// 根据 DataAsset 列表生成武器实例并装备主武器。
	void InitializeWeapons(const TArray<UTSWeaponDataAsset*>& WeaponDataAssets);

	void EquipWeapon(int32 SlotIndex);
	void AttachWeaponToHand();
	void SwitchToSlot(int32 SlotIndex);
	void SwitchNext();

	ATSWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }
	ATSWeaponBase* GetWeaponInSlot(int32 SlotIndex) const;

	// 切枪能力激活前写入目标槽位（-2 = 下一个）。
	void SetPendingSwitchSlot(int32 SlotIndex) { PendingSwitchSlot = SlotIndex; }
	int32 GetPendingSwitchSlot() const { return PendingSwitchSlot; }
	void ClearPendingSwitchSlot() { PendingSwitchSlot = INDEX_NONE; }

	UPROPERTY(BlueprintAssignable, Category = "TS|Weapon")
	FOnWeaponChanged OnWeaponChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TArray<TObjectPtr<ATSWeaponBase>> WeaponSlots;

	UPROPERTY()
	TObjectPtr<ATSWeaponBase> CurrentWeapon;

	UPROPERTY()
	int32 CurrentSlotIndex = 0;

	// 手部附加 socket 名。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Weapon")
	FName HandSocketName = TEXT("hand_r");

	int32 PendingSwitchSlot = INDEX_NONE;

	void HideAllWeapons();
};
