// 玩家角色：COD 式越肩相机 + Enhanced Input（Move/Look/Jump 直接调用，战斗/移动能力经 InputTag 路由到 ASC）。

#pragma once

#include "CoreMinimal.h"
#include "Character/TSCharacterBase.h"
#include "GameplayTagContainer.h"
#include "TSPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UTSInputConfig;
struct FInputActionValue;

UCLASS()
class AI_TPSDEMO2_API ATSPlayerCharacter : public ATSCharacterBase
{
	GENERATED_BODY()

public:
	ATSPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void Tick(float DeltaSeconds) override;

	UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

protected:
	virtual void GrantDefaultAbilities() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// 原生移动/视角输入
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Crouch();

	// 能力输入路由
	void Input_AbilityTagPressed(FGameplayTag InputTag);
	void Input_AbilityTagReleased(FGameplayTag InputTag);
	void RequestWeaponSwitch(int32 SlotIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TS|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TS|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// 鼠标视角映射（与 DefaultMappingContext 叠加，优先级更高）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Input")
	TObjectPtr<UInputAction> LookAction;

	// 鼠标 Look 输入（IA_MouseLook，与手柄 Look 分开映射）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Input")
	TObjectPtr<UInputAction> CrouchAction;

	// 战斗/移动能力的输入映射表（IA ↔ Ability InputTag）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Input")
	TObjectPtr<UTSInputConfig> InputConfig;

	// DEMO 调试：屏幕显示弹药/Muzzle 信息（验收截图用）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Debug")
	bool bShowCombatDebug = true;
};
