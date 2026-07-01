// HUD 控件基类实现（模块9）。

#include "UI/TSUserWidget.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "Core/TSAttributeSet.h"
#include "Core/TSGameplayTags.h"
#include "Character/TSCharacterBase.h"
#include "Weapon/TSCombatComponent.h"
#include "Weapon/TSWeaponBase.h"
#include "Weapon/TSWeaponDataAsset.h"

void UTSUserWidget::InitializeForPlayer(APlayerController* InController)
{
	OwningController = InController;
	RebindToOwner();
}

void UTSUserWidget::RebindToOwner()
{
	// 先解绑旧委托，避免重复绑定或悬垂。
	UnbindAll();

	APlayerController* Controller = OwningController.Get();
	if (!Controller)
	{
		return;
	}

	ATSCharacterBase* Character = Cast<ATSCharacterBase>(Controller->GetPawn());
	OwningCharacter = Character;
	if (!Character)
	{
		return;
	}

	ASC = Character->GetAbilitySystemComponent();
	CombatComponent = Character->GetCombatComponent();

	// 血量属性变化绑定。
	if (UAbilitySystemComponent* ASCPtr = ASC.Get())
	{
		HealthChangedHandle = ASCPtr->GetGameplayAttributeValueChangeDelegate(UTSAttributeSet::GetHealthAttribute())
			.AddUObject(this, &UTSUserWidget::HandleHealthChanged);

		// MaxHealth 变化（可选）：复用同一处理逻辑刷新百分比。
		MaxHealthChangedHandle = ASCPtr->GetGameplayAttributeValueChangeDelegate(UTSAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &UTSUserWidget::HandleHealthChanged);

		// 同步血量初值。
		if (const UTSAttributeSet* AttrSet = Character->GetAttributeSet())
		{
			OnHealthChanged(AttrSet->GetHealth(), AttrSet->GetMaxHealth());
		}
	}

	// 武器切换绑定。
	if (UTSCombatComponent* Combat = CombatComponent.Get())
	{
		Combat->OnWeaponChanged.AddDynamic(this, &UTSUserWidget::HandleWeaponChanged);
	}

	// 绑定当前武器弹药并同步武器/弹药初值。
	BindAmmoToCurrentWeapon();

	const int32 ClipAmmo = GetCurrentClipAmmo();
	const int32 ReserveAmmo = GetCurrentReserveAmmo();
	LastClipAmmo = ClipAmmo;
	OnWeaponChanged(GetCurrentWeaponName(), ClipAmmo, ReserveAmmo);
	OnAmmoChanged(ClipAmmo, ReserveAmmo);
}

void UTSUserWidget::UnbindAll()
{
	if (UAbilitySystemComponent* ASCPtr = ASC.Get())
	{
		if (HealthChangedHandle.IsValid())
		{
			ASCPtr->GetGameplayAttributeValueChangeDelegate(UTSAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		}
		if (MaxHealthChangedHandle.IsValid())
		{
			ASCPtr->GetGameplayAttributeValueChangeDelegate(UTSAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		}
	}
	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();

	if (UTSCombatComponent* Combat = CombatComponent.Get())
	{
		Combat->OnWeaponChanged.RemoveDynamic(this, &UTSUserWidget::HandleWeaponChanged);
	}

	if (ATSWeaponBase* Weapon = BoundWeapon.Get())
	{
		Weapon->OnAmmoChanged.RemoveDynamic(this, &UTSUserWidget::HandleAmmoChanged);
	}
	BoundWeapon = nullptr;
}

void UTSUserWidget::BindAmmoToCurrentWeapon()
{
	// 先解绑旧武器弹药委托。
	if (ATSWeaponBase* OldWeapon = BoundWeapon.Get())
	{
		OldWeapon->OnAmmoChanged.RemoveDynamic(this, &UTSUserWidget::HandleAmmoChanged);
	}
	BoundWeapon = nullptr;

	UTSCombatComponent* Combat = CombatComponent.Get();
	if (!Combat)
	{
		return;
	}

	if (ATSWeaponBase* NewWeapon = Combat->GetCurrentWeapon())
	{
		NewWeapon->OnAmmoChanged.AddDynamic(this, &UTSUserWidget::HandleAmmoChanged);
		BoundWeapon = NewWeapon;
	}
}

void UTSUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 若控制器已设置（InitializeForPlayer 先于构造的情况），补一次绑定。
	if (OwningController.IsValid())
	{
		RebindToOwner();
	}
}

void UTSUserWidget::NativeDestruct()
{
	UnbindAll();
	Super::NativeDestruct();
}

void UTSUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 开火脉冲随时间回落到 0。
	FirePulse = FMath::FInterpTo(FirePulse, 0.f, InDeltaTime, FirePulseInterpSpeed);
}

void UTSUserWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	float NewHealth = 0.f;
	float MaxHealth = 0.f;
	if (const ATSCharacterBase* Character = OwningCharacter.Get())
	{
		if (const UTSAttributeSet* AttrSet = Character->GetAttributeSet())
		{
			NewHealth = AttrSet->GetHealth();
			MaxHealth = AttrSet->GetMaxHealth();
		}
	}
	OnHealthChanged(NewHealth, MaxHealth);
}

void UTSUserWidget::HandleWeaponChanged()
{
	// 重绑弹药委托到新当前武器。
	BindAmmoToCurrentWeapon();

	const int32 ClipAmmo = GetCurrentClipAmmo();
	const int32 ReserveAmmo = GetCurrentReserveAmmo();
	LastClipAmmo = ClipAmmo;

	OnWeaponChanged(GetCurrentWeaponName(), ClipAmmo, ReserveAmmo);
	OnAmmoChanged(ClipAmmo, ReserveAmmo);
}

void UTSUserWidget::HandleAmmoChanged(int32 ClipAmmo, int32 ReserveAmmo)
{
	// 弹匣较上次减少视为开火，触发准星脉冲。
	if (ClipAmmo < LastClipAmmo)
	{
		TriggerFirePulse();
	}
	LastClipAmmo = ClipAmmo;

	OnAmmoChanged(ClipAmmo, ReserveAmmo);
}

void UTSUserWidget::TriggerFirePulse()
{
	FirePulse = FirePulseAdd;
}

float UTSUserWidget::GetHealthPercent() const
{
	if (const ATSCharacterBase* Character = OwningCharacter.Get())
	{
		if (const UTSAttributeSet* AttrSet = Character->GetAttributeSet())
		{
			const float MaxHealth = AttrSet->GetMaxHealth();
			if (MaxHealth > 0.f)
			{
				return FMath::Clamp(AttrSet->GetHealth() / MaxHealth, 0.f, 1.f);
			}
		}
	}
	return 0.f;
}

int32 UTSUserWidget::GetCurrentClipAmmo() const
{
	if (const UTSCombatComponent* Combat = CombatComponent.Get())
	{
		if (const ATSWeaponBase* Weapon = Combat->GetCurrentWeapon())
		{
			return Weapon->GetCurrentAmmo();
		}
	}
	return 0;
}

int32 UTSUserWidget::GetCurrentReserveAmmo() const
{
	if (const UTSCombatComponent* Combat = CombatComponent.Get())
	{
		if (const ATSWeaponBase* Weapon = Combat->GetCurrentWeapon())
		{
			return Weapon->GetCurrentReserve();
		}
	}
	return 0;
}

FName UTSUserWidget::GetCurrentWeaponName() const
{
	if (const UTSCombatComponent* Combat = CombatComponent.Get())
	{
		if (const ATSWeaponBase* Weapon = Combat->GetCurrentWeapon())
		{
			if (const UTSWeaponDataAsset* Data = Weapon->GetWeaponData())
			{
				return Data->WeaponName;
			}
		}
	}
	return NAME_None;
}

bool UTSUserWidget::IsADS() const
{
	if (const UAbilitySystemComponent* ASCPtr = ASC.Get())
	{
		return ASCPtr->HasMatchingGameplayTag(TSGameplayTags::State_Combat_ADS);
	}
	return false;
}

bool UTSUserWidget::IsSprinting() const
{
	if (const UAbilitySystemComponent* ASCPtr = ASC.Get())
	{
		return ASCPtr->HasMatchingGameplayTag(TSGameplayTags::State_Movement_Sprinting);
	}
	return false;
}

float UTSUserWidget::GetCrosshairGapPixels() const
{
	// ADS 时准星收紧为固定小间隙。
	if (IsADS())
	{
		return 2.f;
	}

	// 基值：冲刺 > 移动 > 静止。
	float Base = 8.f;
	if (IsSprinting())
	{
		Base = 28.f;
	}
	else if (const ATSCharacterBase* Character = OwningCharacter.Get())
	{
		if (Character->GetVelocity().Size2D() > 10.f)
		{
			Base = 16.f;
		}
	}

	// 叠加开火脉冲。
	return Base + FirePulse;
}
