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
		return FMath::Max(SprintSpeed, SlideEnterImpulse);
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

	const float EnterSpeed = FMath::Max(Velocity.Size2D(), SlideEnterImpulse);
	Velocity = SlideDir * EnterSpeed;
	SlideStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UTSCharacterMovementComponent::EndSlide()
{
	if (!IsSliding())
	{
		return;
	}

	SetMovementMode(MOVE_Walking);
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

	const FVector Start = UpdatedComponent->GetComponentLocation();
	const float HalfHeight = CharacterOwner->GetCapsuleComponent()
		? CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		: 44.f;
	const FVector End = Start - FVector::UpVector * (HalfHeight + 50.f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CharacterOwner);
	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}

void UTSCharacterMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < 1e-4f)
	{
		return;
	}

	const float Elapsed = (GetWorld() ? GetWorld()->GetTimeSeconds() : SlideStartTime) - SlideStartTime;
	FHitResult SurfaceHit;

	// 结束条件：超时 / 速度过低 / 离地。
	if (Elapsed >= SlideDuration
		|| Velocity.SizeSquared2D() < FMath::Square(SlideMinSpeed)
		|| !GetSlideSurface(SurfaceHit))
	{
		EndSlide();
		StartNewPhysics(DeltaTime, Iterations);
		return;
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
