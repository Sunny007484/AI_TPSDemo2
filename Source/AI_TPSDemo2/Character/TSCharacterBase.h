// 角色基类：承载 ASC / AttributeSet，定义 ASC 初始化与默认能力/效果授予流程（模块2）。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "TSCharacterBase.generated.h"

class UTSAbilitySystemComponent;
class UTSAttributeSet;
class UTSWeaponAttributeSet;
class UTSGameplayAbility;
class UGameplayEffect;
class UTSCombatComponent;
class UTSWeaponDataAsset;

UCLASS(Abstract)
class AI_TPSDEMO2_API ATSCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATSCharacterBase(const FObjectInitializer& ObjectInitializer);

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	UTSAbilitySystemComponent* GetTSAbilitySystemComponent() const { return AbilitySystemComponent; }
	UTSAttributeSet* GetAttributeSet() const { return AttributeSet; }
	UTSCombatComponent* GetCombatComponent() const { return CombatComponent; }

	// 是否已死亡（含 State.Dead tag），细节在模块6 完善。
	UFUNCTION(BlueprintPure, Category = "TS|State")
	virtual bool IsDeadOrDying() const;

	// 死亡处理入口，模块6 实现 Ragdoll/取消能力/重生等。
	virtual void HandleDeath();

	// 滑铲中跳跃会中断滑铲并保留速度起跳。
	virtual void Jump() override;

	//~ 蹲伏状态与 GameplayTag 同步
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

protected:
	// 死亡后续处理钩子：基类空实现，子类覆写（玩家重生 / AI 清理）。
	virtual void OnDeath();

	// 死亡时缓存的控制器（UnPossess 后 GetController 失效，子类重生/清理时使用）。
	AController* GetDeathController() const { return DeathController; }

	// ASC 初始化（Player 在 PossessedBy + OnRep_PlayerState；AI 在 PossessedBy 调用）。
	virtual void InitAbilityActorInfo();

	virtual void GrantDefaultAbilities();
	void ApplyDefaultEffects();
	void InitializeDefaultWeapons();

	UPROPERTY(VisibleAnywhere, Category = "TS|Abilities")
	TObjectPtr<UTSAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UTSAttributeSet> AttributeSet;

	UPROPERTY()
	TObjectPtr<UTSWeaponAttributeSet> WeaponAttributeSet;

	// 角色启动时授予的能力（蓝图配置）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Abilities")
	TArray<TSubclassOf<UTSGameplayAbility>> DefaultAbilities;

	// 角色启动时施加的效果（初始化属性，蓝图配置 GE_InitAttributes 等）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	// 默认武器 DataAsset（Primary + Secondary）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Weapon")
	TArray<TObjectPtr<UTSWeaponDataAsset>> DefaultWeapons;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TS|Weapon")
	TObjectPtr<UTSCombatComponent> CombatComponent;

	// 是否已进入死亡流程（与 State.Dead tag 保持一致，用于防重入）。
	bool bDead = false;

	// 死亡瞬间缓存的控制器。
	UPROPERTY()
	TObjectPtr<AController> DeathController;
};
