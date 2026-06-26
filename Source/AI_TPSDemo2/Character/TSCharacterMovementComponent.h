// 自定义角色移动组件：Walk/Sprint/Crouch 速度 + 自定义滑铲模式 CMOVE_Slide（模块3）。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TSCharacterMovementComponent.generated.h"

// 自定义移动模式枚举（配合 MOVE_Custom 使用）。
UENUM(BlueprintType)
enum ETSCustomMovementMode : int
{
	CMOVE_None  UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_MAX   UMETA(Hidden)
};

UCLASS()
class AI_TPSDEMO2_API UTSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UTSCharacterMovementComponent();

	virtual float GetMaxSpeed() const override;

	// 冲刺意图（由 GA_Sprint 设置）。
	void SetWantsToSprint(bool bNewWantsToSprint) { bWantsToSprint = bNewWantsToSprint; }
	bool WantsToSprint() const { return bWantsToSprint; }

	// ADS 移速压制（由 GA_ADS 设置，模块5）。
	void SetAimingDownSights(bool bNewAiming) { bAimingDownSights = bNewAiming; }

	// 滑铲控制（由 GA_Slide 调用）。
	void StartSlide();
	void EndSlide();
	bool IsSliding() const;
	float GetSlideDuration() const { return SlideDuration; }

	// 当前是否朝角色前向移动（冲刺仅前向有效）。
	bool IsMovingForward() const;

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	void PhysSlide(float DeltaTime, int32 Iterations);
	bool GetSlideSurface(FHitResult& OutHit) const;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Movement")
	float SprintSpeed = 750.f;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Movement")
	float ADSSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideEnterImpulse = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideMinSpeed = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideDuration = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideFriction = 0.5f;

private:
	bool bWantsToSprint = false;
	bool bAimingDownSights = false;
	float SlideStartTime = 0.f;
};
