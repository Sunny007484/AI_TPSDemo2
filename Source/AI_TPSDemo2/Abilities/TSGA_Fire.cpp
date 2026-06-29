#include "Abilities/TSGA_Fire.h"

#include "Abilities/TSGE_Damage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/TSCharacterMovementComponent.h"
#include "Character/TSPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/TSCollisionChannels.h"
#include "Core/TSAbilitySystemComponent.h"
#include "Core/TSGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Weapon/TSCombatComponent.h"
#include "Weapon/TSWeaponBase.h"
#include "Weapon/TSWeaponDataAsset.h"

UTSGA_Fire::UTSGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = ETSActivationPolicy::WhileInputActive;
	InputTag = TSGameplayTags::Ability_Fire;

	ActivationOwnedTags.AddTag(TSGameplayTags::State_Combat_Firing);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Combat_Reloading);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Weapon_Switching);
	ActivationBlockedTags.AddTag(TSGameplayTags::State_Dead);

	DamageEffectClass = UTSGE_Damage::StaticClass();
}

void UTSGA_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 弹匣已空但仍有备弹：按开火直接自动换弹。
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	if (Weapon && !Weapon->CanFire() && Weapon->GetCurrentAmmo() == 0 && Weapon->GetCurrentReserve() > 0)
	{
		TryTriggerAutoReload();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ConsecutiveShots = 0;
	FireOnce();
}

void UTSGA_Fire::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTSGA_Fire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}

	ConsecutiveShots = 0;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTSGA_Fire::FireOnce()
{
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	if (!Weapon || !Weapon->CanFire())
	{
		if (Weapon && Weapon->GetCurrentAmmo() == 0 && Weapon->GetCurrentReserve() > 0)
		{
			TryTriggerAutoReload();
		}
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	float DamageDealt = 0.f;
	const bool bHit = PerformFireTrace(DamageDealt);

	Weapon->ConsumeAmmo();
	ApplyRecoil();
	PlayFireFeedback();
	++ConsecutiveShots;

	if (bHit && CurrentActorInfo && CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayEventData EventData;
		EventData.Instigator = GetAvatarActorFromActorInfo();
		EventData.Target = GetAvatarActorFromActorInfo();
		EventData.EventMagnitude = DamageDealt;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetAvatarActorFromActorInfo(), TSGameplayTags::Event_Combat_Hit, EventData);
	}

	// 打完最后一发：弹匣空且仍有备弹则自动换弹。
	if (Weapon->GetCurrentAmmo() == 0 && Weapon->GetCurrentReserve() > 0)
	{
		TryTriggerAutoReload();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// 半自动武器：单发后结束；自动武器在按住期间循环。
	if (const UTSWeaponDataAsset* Data = Weapon->GetWeaponData())
	{
		if (!Data->bAutomatic)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}
	}

	ScheduleNextShot();
}

void UTSGA_Fire::TryTriggerAutoReload() const
{
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	const ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	if (!Weapon || Weapon->GetCurrentAmmo() != 0 || Weapon->GetCurrentReserve() <= 0)
	{
		return;
	}

	if (UTSAbilitySystemComponent* ASC = Cast<UTSAbilitySystemComponent>(
		CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr))
	{
		ASC->AbilityInputTagPressed(TSGameplayTags::Ability_Reload);
	}
}

void UTSGA_Fire::ScheduleNextShot()
{
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	if (!Weapon || !Weapon->GetWeaponData())
	{
		return;
	}

	const float FireRate = FMath::Max(Weapon->GetWeaponData()->FireRate, 0.01f);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FireTimerHandle, this, &UTSGA_Fire::OnFireTimerElapsed,
			FireRate, false);
	}
}

void UTSGA_Fire::OnFireTimerElapsed()
{
	FireOnce();
}

bool UTSGA_Fire::PerformFireTrace(float& OutDamage) const
{
	OutDamage = 0.f;

	const ATSPlayerCharacter* PlayerChar = GetTSPlayerCharacterFromActorInfo();
	if (!PlayerChar)
	{
		return false;
	}

	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	const UTSWeaponDataAsset* WeaponData = Weapon ? Weapon->GetWeaponData() : nullptr;
	if (!WeaponData)
	{
		return false;
	}

	UCameraComponent* Camera = PlayerChar->GetFollowCamera();
	if (!Camera)
	{
		return false;
	}

	const FVector TraceStart = Camera->GetComponentLocation();
	FVector TraceDir = Camera->GetForwardVector();

	// 散布：ADS 用 SpreadADS，腰射用 SpreadHip；移动中腰射 ×1.5。
	const float SpreadDeg = GetCurrentSpreadDegrees();
	const FRotator SpreadRot(
		FMath::RandRange(-SpreadDeg, SpreadDeg),
		FMath::RandRange(-SpreadDeg, SpreadDeg),
		0.f);
	TraceDir = SpreadRot.RotateVector(TraceDir).GetSafeNormal();

	const FVector TraceEnd = TraceStart + TraceDir * WeaponData->MaxRange;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TSWeaponFire), true, GetAvatarActorFromActorInfo());
	QueryParams.bReturnPhysicalMaterial = false;

	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd,
		TS_TRACE_WEAPON, QueryParams);

#if ENABLE_DRAW_DEBUG
	DrawDebugLine(GetWorld(), TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd,
		bHit ? FColor::Red : FColor::Green, false, 1.f, 0, 1.f);
	if (bHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 8.f, 8, FColor::Yellow, false, 1.f);
	}
#endif

	if (!bHit)
	{
		return false;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!TargetASC || !DamageEffectClass)
	{
		return false;
	}

	UAbilitySystemComponent* SourceASC = CurrentActorInfo->AbilitySystemComponent.Get();
	if (!SourceASC)
	{
		return false;
	}

	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	if (ACharacter* SourceChar = GetCharacterFromActorInfo())
	{
		if (AController* Controller = SourceChar->GetController())
		{
			ContextHandle.AddInstigator(SourceChar, Controller);
		}
	}

	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		DamageEffectClass, GetAbilityLevel(), ContextHandle);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(TSGameplayTags::Data_Damage, WeaponData->Damage);
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	OutDamage = WeaponData->Damage;
	return true;
}

float UTSGA_Fire::GetCurrentSpreadDegrees() const
{
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	const UTSWeaponDataAsset* WeaponData = Weapon ? Weapon->GetWeaponData() : nullptr;
	if (!WeaponData)
	{
		return 0.f;
	}

	const UAbilitySystemComponent* ASC = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
	const bool bADS = ASC && ASC->HasMatchingGameplayTag(TSGameplayTags::State_Combat_ADS);

	float Spread = bADS ? WeaponData->SpreadADS : WeaponData->SpreadHip;

	if (!bADS)
	{
		if (const ACharacter* Character = GetCharacterFromActorInfo())
		{
			if (const UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
			{
				if (MoveComp->Velocity.Size2D() > 10.f)
				{
					Spread *= 1.5f;
				}
			}
		}
	}

	return Spread;
}

void UTSGA_Fire::ApplyRecoil()
{
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	const UTSWeaponDataAsset* WeaponData = Weapon ? Weapon->GetWeaponData() : nullptr;

	ACharacter* Character = GetCharacterFromActorInfo();
	if (!Character || !WeaponData)
	{
		return;
	}

	float RecoilPitch = 0.5f;
	float RecoilYaw = FMath::RandRange(-0.15f, 0.15f);

	if (WeaponData->RecoilCurve)
	{
		const float TimeVal = static_cast<float>(ConsecutiveShots);
		RecoilPitch = WeaponData->RecoilCurve->GetFloatValue(TimeVal);
	}

	if (AController* Controller = Character->GetController())
	{
		FRotator NewRotation = Controller->GetControlRotation();
		// 垂直后坐力：沿世界 Z 正向抬枪（Pitch 正向）。
		NewRotation += FRotator(RecoilPitch, RecoilYaw, 0.f);
		Controller->SetControlRotation(NewRotation);
	}
}

void UTSGA_Fire::PlayFireFeedback()
{
	UTSCombatComponent* Combat = GetCombatComponentFromActorInfo();
	ATSWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
	const UTSWeaponDataAsset* WeaponData = Weapon ? Weapon->GetWeaponData() : nullptr;
	if (!Weapon || !WeaponData)
	{
		return;
	}

	if (WeaponData->FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, Weapon->GetMuzzleLocation());
	}

	if (WeaponData->MuzzleFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, WeaponData->MuzzleFX,
			Weapon->GetMuzzleLocation(), Weapon->GetActorRotation());
	}

	if (WeaponData->FireMontage)
	{
		if (ACharacter* Character = GetCharacterFromActorInfo())
		{
			if (USkeletalMeshComponent* Mesh = Character->GetMesh())
			{
				if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
				{
					AnimInstance->Montage_Play(WeaponData->FireMontage);
				}
			}
		}
	}
}
