#include "Character/TSCharacterBase.h"
#include "Weapon/TSWeaponBase.h"
#include "Weapon/TSWeaponTypes.h"

#include "AbilitySystemComponent.h"
#include "Character/TSCharacterMovementComponent.h"
#include "Core/TSAbilitySystemComponent.h"
#include "Core/TSAttributeSet.h"
#include "Core/TSGameplayAbility.h"
#include "Core/TSGameplayTags.h"
#include "Core/TSWeaponAttributeSet.h"
#include "Core/TSCollisionChannels.h"
#include "Weapon/TSCombatComponent.h"
#include "Weapon/TSWeaponDataAsset.h"

ATSCharacterBase::ATSCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UTSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UTSAttributeSet>(TEXT("AttributeSet"));
	WeaponAttributeSet = CreateDefaultSubobject<UTSWeaponAttributeSet>(TEXT("WeaponAttributeSet"));

	CombatComponent = CreateDefaultSubobject<UTSCombatComponent>(TEXT("CombatComponent"));

	// 武器射线通道：角色网格需响应 Weapon trace。
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionResponseToChannel(TS_TRACE_WEAPON, ECR_Block);
	}
}

UAbilitySystemComponent* ATSCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ATSCharacterBase::InitAbilityActorInfo()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (HasAuthority())
	{
		ApplyDefaultEffects();
		GrantDefaultAbilities();
		InitializeDefaultWeapons();
	}
}

void ATSCharacterBase::InitializeDefaultWeapons()
{
	if (!CombatComponent || !HasAuthority())
	{
		return;
	}

	TArray<UTSWeaponDataAsset*> WeaponDataAssets;
	WeaponDataAssets.Reserve(DefaultWeapons.Num());
	for (UTSWeaponDataAsset* DataAsset : DefaultWeapons)
	{
		if (DataAsset)
		{
			WeaponDataAssets.Add(DataAsset);
		}
	}

	// 无蓝图配置时使用 DEMO 默认武器数据（模块4 规格表）。
	if (WeaponDataAssets.Num() == 0)
	{
		UTSWeaponDataAsset* ARData = NewObject<UTSWeaponDataAsset>(this, TEXT("DemoARData"));
		ARData->WeaponName = FName(TEXT("AssaultRifle"));
		ARData->Slot = ETSWeaponSlot::Primary;
		ARData->WeaponClass = ATSWeaponBase::StaticClass();
		ARData->Damage = 28.f;
		ARData->FireRate = 0.10f;
		ARData->bAutomatic = true;
		ARData->ClipSize = 30;
		ARData->MaxReserveAmmo = 180;
		ARData->ReloadTime = 2.3f;
		ARData->SpreadHip = 3.f;
		ARData->SpreadADS = 0.5f;
		ARData->MaxRange = 10000.f;
		ARData->ADS_FOV = 55.f;
		ARData->ADS_CameraOffset = FVector(0.f, 30.f, 40.f);
		WeaponDataAssets.Add(ARData);

		UTSWeaponDataAsset* PistolData = NewObject<UTSWeaponDataAsset>(this, TEXT("DemoPistolData"));
		PistolData->WeaponName = FName(TEXT("Pistol"));
		PistolData->Slot = ETSWeaponSlot::Secondary;
		PistolData->WeaponClass = ATSWeaponBase::StaticClass();
		PistolData->Damage = 45.f;
		PistolData->FireRate = 0.20f;
		PistolData->bAutomatic = false;
		PistolData->ClipSize = 12;
		PistolData->MaxReserveAmmo = 60;
		PistolData->ReloadTime = 1.5f;
		PistolData->SpreadHip = 2.f;
		PistolData->SpreadADS = 0.4f;
		PistolData->MaxRange = 8000.f;
		PistolData->ADS_FOV = 60.f;
		PistolData->ADS_CameraOffset = FVector(0.f, 25.f, 35.f);
		WeaponDataAssets.Add(PistolData);
	}

	CombatComponent->InitializeWeapons(WeaponDataAssets);
}

void ATSCharacterBase::GrantDefaultAbilities()
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	TArray<TSubclassOf<UGameplayAbility>> Abilities;
	Abilities.Reserve(DefaultAbilities.Num());
	for (const TSubclassOf<UTSGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			Abilities.Add(AbilityClass);
		}
	}

	AbilitySystemComponent->GrantStartupAbilities(Abilities);
}

void ATSCharacterBase::ApplyDefaultEffects()
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		const FGameplayEffectSpecHandle SpecHandle =
			AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

bool ATSCharacterBase::IsDeadOrDying() const
{
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(TSGameplayTags::State_Dead);
}

void ATSCharacterBase::Jump()
{
	if (UTSCharacterMovementComponent* MoveComp = Cast<UTSCharacterMovementComponent>(GetCharacterMovement()))
	{
		if (MoveComp->IsSliding())
		{
			MoveComp->InterruptSlideForJump();
		}
	}

	Super::Jump();
}

void ATSCharacterBase::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(TSGameplayTags::State_Movement_Crouching);
	}
}

void ATSCharacterBase::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(TSGameplayTags::State_Movement_Crouching);
	}
}

void ATSCharacterBase::HandleDeath()
{
	// 模块6 实现：取消能力、Ragdoll、AI 停 BT、玩家延时重生。
}
