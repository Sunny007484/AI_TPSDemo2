#include "Character/TSCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

UTSCharacterMovementComponent::UTSCharacterMovementComponent()
{
	MaxWalkSpeed = 450.f;
	MaxWalkSpeedCrouched = 250.f;
	NavAgentProps.bCanCrouch = true;
	bCanWalkOffLedgesWhenCrouching = true;
}

float UTSCharacterMovementComponent::GetMaxSpeed() const
{
	if (IsSliding())
	{
		return SlideSpeed;
	}

	if (MovementMode == MOVE_Walking && !IsCrouching())
	{
		if (bAimingDownSights)
		{
			return ADSSpeed;
		}
		if (bWantsToSprint && IsMovingForward())
		{
			return SprintSpeed;
		}
	}

	return Super::GetMaxSpeed();
}

bool UTSCharacterMovementComponent::IsMovingForward() const
{
	if (!CharacterOwner)
	{
		return false;
	}

	const FVector Accel = GetCurrentAcceleration();
	if (Accel.IsNearlyZero())
	{
		return false;
	}

	const FVector Forward = CharacterOwner->GetActorForwardVector();
	return FVector::DotProduct(Forward.GetSafeNormal2D(), Accel.GetSafeNormal2D()) > 0.5f;
}

bool UTSCharacterMovementComponent::IsSliding() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Slide;
}

void UTSCharacterMovementComponent::StartSlide()
{
	if (IsSliding() || !CharacterOwner)
	{
		return;
	}

	// 降低 capsule（复用引擎蹲伏机制，半高 88→44）。
	CharacterOwner->Crouch();

	FVector SlideDir = Velocity.GetSafeNormal2D();
	if (SlideDir.IsNearlyZero())
	{
		SlideDir = CharacterOwner->GetActorForwardVector().GetSafeNormal2D();
	}

	const float EnterSpeed = FMath::Max(Velocity.Size2D(), SlideSpeed);
	Velocity = SlideDir * EnterSpeed;
	SlideStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	bSlideAirGapActive = false;
	SlideAirGapVelocity = FVector::ZeroVector;

	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UTSCharacterMovementComponent::EndSlide(bool bPreserveVelocity)
{
	if (!IsSliding())
	{
		return;
	}

	bSlideAirGapActive = false;
	SlideAirGapVelocity = FVector::ZeroVector;
	SetMovementMode(MOVE_Walking);

	if (!bPreserveVelocity)
	{
		// 正常退出：将水平速度钳制回常规行走上限，避免惯性残留。
		const float Speed2D = Velocity.Size2D();
		if (Speed2D > MaxWalkSpeed)
		{
			Velocity = Velocity.GetSafeNormal2D() * MaxWalkSpeed;
		}
	}

	OnSlideMovementEnded.Broadcast();
}

void UTSCharacterMovementComponent::InterruptSlideForJump()
{
	if (!IsSliding() || !CharacterOwner)
	{
		return;
	}

	bSlideJumpDecelerating = true;
	bSlideAirGapActive = false;
	SlideAirGapVelocity = FVector::ZeroVector;

	// 起跳前必须起身，否则 CanJump 为 false。
	if (CharacterOwner->bIsCrouched)
	{
		CharacterOwner->UnCrouch();
	}

	EndSlide(/*bPreserveVelocity=*/true);
}

float UTSCharacterMovementComponent::GetSlideJumpTargetSpeed() const
{
	return bWantsToSprint ? SprintSpeed : MaxWalkSpeed;
}

void UTSCharacterMovementComponent::ApplySlideJumpDeceleration(float DeltaTime)
{
	const float TargetSpeed = GetSlideJumpTargetSpeed();
	const float Speed2D = Velocity.Size2D();

	if (Speed2D <= TargetSpeed + KINDA_SMALL_NUMBER)
	{
		if (Speed2D > KINDA_SMALL_NUMBER)
		{
			const float VerticalSpeed = Velocity.Z;
			Velocity = Velocity.GetSafeNormal2D() * TargetSpeed;
			Velocity.Z = VerticalSpeed;
		}
		bSlideJumpDecelerating = false;
		return;
	}

	const float NewSpeed = FMath::FInterpTo(Speed2D, TargetSpeed, DeltaTime, SlideJumpDecelerationRate);
	const float VerticalSpeed = Velocity.Z;
	Velocity = Velocity.GetSafeNormal2D() * NewSpeed;
	Velocity.Z = VerticalSpeed;
}

void UTSCharacterMovementComponent::PhysFalling(float DeltaTime, int32 Iterations)
{
	Super::PhysFalling(DeltaTime, Iterations);

	if (bSlideJumpDecelerating)
	{
		ApplySlideJumpDeceleration(DeltaTime);
	}
}

void UTSCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Falling && IsMovingOnGround())
	{
		bSlideJumpDecelerating = false;
	}
}

void UTSCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (CustomMovementMode == CMOVE_Slide)
	{
		PhysSlide(DeltaTime, Iterations);
		return;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

bool UTSCharacterMovementComponent::GetSlideSurface(FHitResult& OutHit) const
{
	if (!CharacterOwner)
	{
		return false;
	}

	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 44.f;
	const FVector CapsuleBottom = UpdatedComponent->GetComponentLocation() - FVector::UpVector * HalfHeight;
	const FVector TraceEnd = CapsuleBottom - FVector::UpVector * 50.f;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SlideFloorTrace), false, CharacterOwner);
	return GetWorld()->LineTraceSingleByChannel(OutHit, CapsuleBottom, TraceEnd, ECC_WorldStatic, Params);
}

void UTSCharacterMovementComponent::ExitSlideToFalling()
{
	if (!IsSliding())
	{
		return;
	}

	bSlideAirGapActive = false;
	SlideAirGapVelocity = FVector::ZeroVector;
	SetMovementMode(MOVE_Falling);
	// 保留当前速度进入 Falling，不做钳制。
	OnSlideMovementEnded.Broadcast();
}

void UTSCharacterMovementComponent::PhysSlideAirGap(float DeltaTime, int32 Iterations)
{
	// 宽限期内保留进入凹陷时的水平速度，仅受重力影响垂直分量。
	Velocity.X = SlideAirGapVelocity.X;
	Velocity.Y = SlideAirGapVelocity.Y;
	Velocity.Z += GetGravityZ() * DeltaTime;

	Iterations++;
	bJustTeleported = false;

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Delta = Velocity * DeltaTime;
	FHitResult MoveHit(1.f);
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, MoveHit);

	if (MoveHit.Time < 1.f)
	{
		HandleImpact(MoveHit, DeltaTime, Delta);
		SlideAlongSurface(Delta, 1.f - MoveHit.Time, MoveHit.Normal, MoveHit, true);
	}

	const float VerticalSpeed = (UpdatedComponent->GetComponentLocation() - OldLocation).Z / DeltaTime;
	Velocity.X = SlideAirGapVelocity.X;
	Velocity.Y = SlideAirGapVelocity.Y;
	Velocity.Z = VerticalSpeed;
}

void UTSCharacterMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < 1e-4f)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : SlideStartTime;
	const float Elapsed = Now - SlideStartTime;
	FHitResult SurfaceHit;
	const bool bHasSurface = GetSlideSurface(SurfaceHit);

	// 超时 / 速度过低：正常结束（在地面）或宽限期已结束时退出到 Falling。
	if (Elapsed >= SlideDuration
		|| Velocity.SizeSquared2D() < FMath::Square(SlideMinSpeed))
	{
		if (bSlideAirGapActive || !bHasSurface)
		{
			ExitSlideToFalling();
		}
		else
		{
			EndSlide();
		}
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	// 地面凹陷：离地后进入宽限期，保留速度与滑铲状态。
	if (!bHasSurface)
	{
		if (!bSlideAirGapActive)
		{
			bSlideAirGapActive = true;
			SlideAirGapStartTime = Now;
			SlideAirGapVelocity = Velocity;
		}

		const float AirGapElapsed = Now - SlideAirGapStartTime;
		if (AirGapElapsed >= SlideAirGapGraceDuration)
		{
			ExitSlideToFalling();
			StartNewPhysics(DeltaTime, Iterations);
			return;
		}

		PhysSlideAirGap(DeltaTime, Iterations);
		return;
	}

	// 宽限期内重新落地：清除宽限，继续正常滑铲直至 SlideDuration 结束。
	if (bSlideAirGapActive)
	{
		bSlideAirGapActive = false;
		SlideAirGapVelocity = FVector::ZeroVector;
	}

	// 速度投影到地表并施加摩擦衰减。
	Velocity = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal);
	Velocity *= FMath::Clamp(1.f - SlideFriction * DeltaTime, 0.f, 1.f);

	Iterations++;
	bJustTeleported = false;

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Delta = Velocity * DeltaTime;
	FHitResult MoveHit(1.f);
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, MoveHit);

	if (MoveHit.Time < 1.f)
	{
		HandleImpact(MoveHit, DeltaTime, Delta);
		SlideAlongSurface(Delta, 1.f - MoveHit.Time, MoveHit.Normal, MoveHit, true);
	}

	// 用实际位移重算速度。
	Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
}
