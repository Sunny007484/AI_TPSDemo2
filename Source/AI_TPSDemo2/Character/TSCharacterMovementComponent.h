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
	void EndSlide(bool bPreserveVelocity = false);
	// 跳跃输入中断滑铲：保留当前速度并进入跳跃，空中逐渐减速至奔跑/行走速度。
	void InterruptSlideForJump();
	bool IsSliding() const;
	float GetSlideDuration() const { return SlideDuration; }

	// 移动组件因超时/摩擦/离地提前结束滑铲时广播（GA_Slide 监听以同步结束能力）。
	DECLARE_MULTICAST_DELEGATE(FOnSlideMovementEnded);
	FOnSlideMovementEnded OnSlideMovementEnded;

	// 当前是否朝角色前向移动（冲刺仅前向有效）。
	bool IsMovingForward() const;

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void PhysFalling(float DeltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	void PhysSlide(float DeltaTime, int32 Iterations);
	void PhysSlideAirGap(float DeltaTime, int32 Iterations);
	void ExitSlideToFalling();
	bool GetSlideSurface(FHitResult& OutHit) const;
	void ApplySlideJumpDeceleration(float DeltaTime);
	float GetSlideJumpTargetSpeed() const;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Movement")
	float SprintSpeed = 750.f;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Movement")
	float ADSSpeed = 300.f;

	// 滑铲进入时的目标水平速度（cm/s）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideSpeed = 2500.f;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideMinSpeed = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideDuration = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideFriction = 0.5f;

	// 滑铲跳跃中断后，空中水平速度向目标速度插值的速率。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideJumpDecelerationRate = 3.f;

	// 滑铲经过地面凹陷离地后，保留速度与滑铲状态的最长宽限时间（秒）。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Slide")
	float SlideAirGapGraceDuration = 0.3f;

private:
	bool bWantsToSprint = false;
	bool bAimingDownSights = false;
	bool bSlideJumpDecelerating = false;
	bool bSlideAirGapActive = false;
	float SlideStartTime = 0.f;
	float SlideAirGapStartTime = 0.f;
	FVector SlideAirGapVelocity = FVector::ZeroVector;
};
