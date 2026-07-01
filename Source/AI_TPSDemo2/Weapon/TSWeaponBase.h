// 武器 Actor：承载网格与运行时弹药（模块4）。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TSWeaponBase.generated.h"

class UTSWeaponDataAsset;
class USkeletalMeshComponent;
class UStaticMeshComponent;

// 弹药变化动态多播（弹匣余弹 / 备弹），供 HUD 等订阅刷新（模块9）。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, ClipAmmo, int32, ReserveAmmo);

UCLASS()
class AI_TPSDEMO2_API ATSWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ATSWeaponBase();

	// 弹药变化广播（InitializeFromData / ConsumeAmmo / ApplyReload 后触发）。
	UPROPERTY(BlueprintAssignable, Category = "TS|Weapon")
	FOnAmmoChanged OnAmmoChanged;

	// 用 DataAsset 初始化运行时弹药。
	void InitializeFromData(UTSWeaponDataAsset* InWeaponData);

	UTSWeaponDataAsset* GetWeaponData() const { return WeaponData; }

	bool CanFire() const;
	void ConsumeAmmo();
	int32 CalcReloadAmount() const;
	void ApplyReload();

	FVector GetMuzzleLocation() const;

	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	int32 GetCurrentReserve() const { return CurrentReserve; }

	// 网格组件（骨骼或静态，蓝图可指定网格）。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TS|Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TS|Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponStaticMesh;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_WeaponData)
	TObjectPtr<UTSWeaponDataAsset> WeaponData;

	UPROPERTY(Replicated)
	int32 CurrentAmmo = 0;

	UPROPERTY(Replicated)
	int32 CurrentReserve = 0;

	UFUNCTION()
	void OnRep_WeaponData();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
