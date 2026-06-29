#include "Weapon/TSCombatComponent.h"

#include "Character/TSCharacterBase.h"
#include "GameFramework/Character.h"
#include "Weapon/TSWeaponBase.h"
#include "Weapon/TSWeaponDataAsset.h"

UTSCombatComponent::UTSCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	WeaponSlots.SetNum(MaxWeaponSlots);
}

void UTSCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTSCombatComponent::InitializeWeapons(const TArray<UTSWeaponDataAsset*>& WeaponDataAssets)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 SlotIndex = 0; SlotIndex < MaxWeaponSlots; ++SlotIndex)
	{
		if (!WeaponDataAssets.IsValidIndex(SlotIndex) || !WeaponDataAssets[SlotIndex])
		{
			continue;
		}

		UTSWeaponDataAsset* DataAsset = WeaponDataAssets[SlotIndex];
		TSubclassOf<ATSWeaponBase> WeaponClass = DataAsset->WeaponClass;
		if (!WeaponClass)
		{
			WeaponClass = ATSWeaponBase::StaticClass();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.Instigator = Cast<APawn>(Owner);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ATSWeaponBase* Weapon = World->SpawnActor<ATSWeaponBase>(WeaponClass, SpawnParams);
		if (!Weapon)
		{
			continue;
		}

		Weapon->InitializeFromData(DataAsset);
		WeaponSlots[SlotIndex] = Weapon;
	}

	if (WeaponSlots[0])
	{
		EquipWeapon(0);
	}
}

void UTSCombatComponent::EquipWeapon(int32 SlotIndex)
{
	if (!WeaponSlots.IsValidIndex(SlotIndex) || !WeaponSlots[SlotIndex])
	{
		return;
	}

	CurrentSlotIndex = SlotIndex;
	CurrentWeapon = WeaponSlots[SlotIndex];
	HideAllWeapons();
	AttachWeaponToHand();
	OnWeaponChanged.Broadcast();
}

void UTSCombatComponent::AttachWeaponToHand()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !CurrentWeapon)
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	CurrentWeapon->AttachToComponent(CharacterMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketName);
	CurrentWeapon->SetActorHiddenInGame(false);
}

void UTSCombatComponent::SwitchToSlot(int32 SlotIndex)
{
	if (SlotIndex == CurrentSlotIndex)
	{
		return;
	}
	if (!WeaponSlots.IsValidIndex(SlotIndex) || !WeaponSlots[SlotIndex])
	{
		return;
	}

	EquipWeapon(SlotIndex);
}

void UTSCombatComponent::SwitchNext()
{
	for (int32 Offset = 1; Offset < MaxWeaponSlots; ++Offset)
	{
		const int32 NextSlot = (CurrentSlotIndex + Offset) % MaxWeaponSlots;
		if (WeaponSlots.IsValidIndex(NextSlot) && WeaponSlots[NextSlot])
		{
			SwitchToSlot(NextSlot);
			return;
		}
	}
}

ATSWeaponBase* UTSCombatComponent::GetWeaponInSlot(int32 SlotIndex) const
{
	return WeaponSlots.IsValidIndex(SlotIndex) ? WeaponSlots[SlotIndex] : nullptr;
}

void UTSCombatComponent::HideAllWeapons()
{
	for (ATSWeaponBase* Weapon : WeaponSlots)
	{
		if (Weapon)
		{
			Weapon->SetActorHiddenInGame(true);
			Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}
	}
}
