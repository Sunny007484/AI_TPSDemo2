// 武器静态配置数据资产（模块4）。

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapon/TSWeaponTypes.h"
#include "TSWeaponDataAsset.generated.h"

class ATSWeaponBase;
class UAnimMontage;
class UCurveFloat;
class UNiagaraSystem;
class USoundBase;

UCLASS(BlueprintType)
class AI_TPSDEMO2_API UTSWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Weapon")
	FName WeaponName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Weapon")
	ETSWeaponSlot Slot = ETSWeaponSlot::Primary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Weapon")
	TSubclassOf<ATSWeaponBase> WeaponClass;

	// 战斗参数
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Combat")
	float Damage = 28.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Combat", meta = (ClampMin = "0.01"))
	float FireRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Combat")
	bool bAutomatic = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Ammo", meta = (ClampMin = "1"))
	int32 ClipSize = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Ammo", meta = (ClampMin = "0"))
	int32 MaxReserveAmmo = 180;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Ammo", meta = (ClampMin = "0.1"))
	float ReloadTime = 2.3f;

	// 散布（度）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Combat")
	float SpreadHip = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Combat")
	float SpreadADS = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|Combat")
	float MaxRange = 10000.f;

	// ADS 相机
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|ADS")
	float ADS_FOV = 55.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|ADS")
	FVector ADS_CameraOffset = FVector(0.f, 30.f, 40.f);

	// 可选美术/反馈资源
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|FX")
	TObjectPtr<UCurveFloat> RecoilCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|FX")
	TObjectPtr<UAnimMontage> FireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|FX")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|FX")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TS|FX")
	TObjectPtr<USoundBase> FireSound;
};
