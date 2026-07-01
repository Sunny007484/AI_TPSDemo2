// 玩家控制器：创建并持有 HUD 控件，处理重生换 pawn 后的重绑（模块9）。
// 注：模块7（命中反馈）后续会在此类追加 GameplayEvent 订阅，故保持结构清晰、便于扩展。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TSPlayerController.generated.h"

class UTSUserWidget;
class USoundBase;
class UAbilitySystemComponent;
struct FGameplayEventData;

UCLASS()
class AI_TPSDEMO2_API ATSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATSPlayerController();

	// 重新 Possess（含重生换 pawn）后让 HUD 重绑到新 pawn，并订阅当前 pawn ASC 的战斗事件（模块7）。
	virtual void OnPossess(APawn* InPawn) override;

	// 失去 pawn 时解绑战斗事件订阅（模块7）。
	virtual void OnUnPossess() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// HUD 控件类（蓝图 WBP 指定）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|HUD")
	TSubclassOf<UTSUserWidget> HUDWidgetClass;

	// 运行时 HUD 控件实例。
	UPROPERTY()
	TObjectPtr<UTSUserWidget> HUDWidget;

	// 击杀音效（可空，模块7）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Feedback")
	TObjectPtr<USoundBase> KillSound;

	// 订阅当前 pawn ASC 的命中/击杀/受伤事件（先解绑旧订阅）。
	void SubscribeCombatEvents(UAbilitySystemComponent* InASC);

	// 解绑已注册的战斗事件订阅。
	void UnsubscribeCombatEvents();

	// 命中事件回调：触发普通命中标记。
	void HandleHitEvent(const FGameplayEventData* Payload);

	// 击杀事件回调：触发击杀命中标记并播放击杀音效。
	void HandleKillEvent(const FGameplayEventData* Payload);

	// 受伤事件回调：按伤害来源位置计算方向角并驱动受伤指示器。
	void HandleTakeDamageEvent(const FGameplayEventData* Payload);

private:
	// 三类战斗事件委托句柄（用于解绑）。
	FDelegateHandle HitEventHandle;
	FDelegateHandle KillEventHandle;
	FDelegateHandle TakeDamageEventHandle;

	// 已订阅的 ASC（弱引用，便于解绑）。
	TWeakObjectPtr<UAbilitySystemComponent> SubscribedASC;

	// 同一射击帧内 Kill 已触发红色标记时，忽略紧随其后的 Hit 事件，避免被白色覆盖。
	bool bSuppressNextHitMarker = false;
};
