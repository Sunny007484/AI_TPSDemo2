// HUD 控件 C++ 基类：缓存所属角色/ASC/战斗组件，绑定血量/武器/弹药委托并向 WBP 推送刷新事件（模块9）。

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TSUserWidget.generated.h"

class APlayerController;
class ATSCharacterBase;
class ATSWeaponBase;
class UAbilitySystemComponent;
class UTSCombatComponent;
struct FOnAttributeChangeData;

/**
 * HUD 控件基类，供各 WBP_ 控件继承。
 * 负责在 C++ 层订阅血量(GAS 属性)、武器切换、弹药变化，并通过 BlueprintImplementableEvent 通知蓝图刷新视觉。
 * 取值采用 BlueprintPure 供蓝图按需读取；准星扩散间隙由 GetCrosshairGapPixels 统一计算。
 */
UCLASS()
class AI_TPSDEMO2_API UTSUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 由 PlayerController 在创建控件后调用：缓存所属控制器并完成首次绑定。
	UFUNCTION(BlueprintCallable, Category = "TS|HUD")
	void InitializeForPlayer(APlayerController* InController);

	// 从 OwningController 当前 Pawn 重新解析角色/ASC/战斗组件并重绑所有委托（重生换 pawn 后调用）。
	UFUNCTION(BlueprintCallable, Category = "TS|HUD")
	void RebindToOwner();

	// 当前血量百分比（Health / MaxHealth，已防除零，结果钳制 0~1）。
	UFUNCTION(BlueprintPure, Category = "TS|HUD")
	float GetHealthPercent() const;

	// 当前武器弹匣余弹。
	UFUNCTION(BlueprintPure, Category = "TS|HUD")
	int32 GetCurrentClipAmmo() const;

	// 当前武器备弹。
	UFUNCTION(BlueprintPure, Category = "TS|HUD")
	int32 GetCurrentReserveAmmo() const;

	// 当前武器名称（取自 DataAsset）。
	UFUNCTION(BlueprintPure, Category = "TS|HUD")
	FName GetCurrentWeaponName() const;

	// 是否处于 ADS 状态（ASC 含 State.Combat.ADS）。
	UFUNCTION(BlueprintPure, Category = "TS|HUD")
	bool IsADS() const;

	// 是否处于冲刺状态（ASC 含 State.Movement.Sprinting）。
	UFUNCTION(BlueprintPure, Category = "TS|HUD")
	bool IsSprinting() const;

	// 准星间隙（像素）：ADS=2；否则按冲刺/移动/静止给基值并叠加开火脉冲。
	UFUNCTION(BlueprintPure, Category = "TS|HUD")
	float GetCrosshairGapPixels() const;

	// 命中标记（模块7）：由 ATSPlayerController 在命中/击杀事件时调用。bIsKill=true 表示击杀（红色加粗样式）。
	UFUNCTION(BlueprintImplementableEvent, Category = "TS|HUD|Feedback")
	void OnHitMarker(bool bIsKill);

	// 受伤方向指示（模块7）：AngleDegrees 为伤害来源相对玩家朝向的角度（0=正前，+90=右，180=后，-90=左）。
	UFUNCTION(BlueprintImplementableEvent, Category = "TS|HUD|Feedback")
	void OnDamageDirection(float AngleDegrees);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 供 WBP 实现的视觉刷新事件。
	UFUNCTION(BlueprintImplementableEvent, Category = "TS|HUD")
	void OnHealthChanged(float NewHealth, float MaxHealth);

	UFUNCTION(BlueprintImplementableEvent, Category = "TS|HUD")
	void OnAmmoChanged(int32 ClipAmmo, int32 ReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, Category = "TS|HUD")
	void OnWeaponChanged(const FName& WeaponName, int32 ClipAmmo, int32 ReserveAmmo);

	// 血量属性变化回调（GAS）。
	void HandleHealthChanged(const FOnAttributeChangeData& Data);

	// 武器切换回调：重绑弹药委托到新武器并同步刷新。
	UFUNCTION()
	void HandleWeaponChanged();

	// 弹药变化回调：弹匣减少时触发开火脉冲，更新记录并通知蓝图。
	UFUNCTION()
	void HandleAmmoChanged(int32 ClipAmmo, int32 ReserveAmmo);

	// 触发一次开火准星脉冲。
	void TriggerFirePulse();

private:
	// 解绑所有已注册的委托（防悬垂）。
	void UnbindAll();

	// 将弹药委托绑定到当前武器（先解绑旧武器）。
	void BindAmmoToCurrentWeapon();

	// 所属控制器。
	TWeakObjectPtr<APlayerController> OwningController;

	// 所属角色。
	TWeakObjectPtr<ATSCharacterBase> OwningCharacter;

	// 角色的 ASC。
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	// 角色的战斗组件。
	TWeakObjectPtr<UTSCombatComponent> CombatComponent;

	// 当前已绑定弹药委托的武器。
	TWeakObjectPtr<ATSWeaponBase> BoundWeapon;

	// 血量属性变化委托句柄（用于解绑）。
	FDelegateHandle HealthChangedHandle;

	// MaxHealth 属性变化委托句柄（可选绑定）。
	FDelegateHandle MaxHealthChangedHandle;

	// 当前开火脉冲量（NativeTick 回落到 0）。
	float FirePulse = 0.f;

	// 上次记录的弹匣余弹（用于判断是否为开火扣弹）。
	int32 LastClipAmmo = 0;

	// 开火脉冲增量与回落参数。
	static constexpr float FirePulseAdd = 12.f;
	static constexpr float FirePulseInterpSpeed = 6.f;
};
