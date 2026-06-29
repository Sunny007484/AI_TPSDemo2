#include "Weapon/TSWeaponBase.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "Weapon/TSWeaponDataAsset.h"

ATSWeaponBase::ATSWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetOnlyOwnerSee(false);

	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticMesh"));
	WeaponStaticMesh->SetupAttachment(WeaponMesh);
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 占位网格：引擎 Cube，缩放为枪形（蓝图/DataAsset 可替换）。
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		WeaponStaticMesh->SetStaticMesh(CubeMesh.Object);
		WeaponStaticMesh->SetRelativeScale3D(FVector(0.08f, 0.04f, 0.25f));
		WeaponStaticMesh->SetRelativeLocation(FVector(15.f, 0.f, 0.f));
		WeaponStaticMesh->SetVisibility(true);
	}
}

void ATSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATSWeaponBase, WeaponData);
	DOREPLIFETIME(ATSWeaponBase, CurrentAmmo);
	DOREPLIFETIME(ATSWeaponBase, CurrentReserve);
}

void ATSWeaponBase::InitializeFromData(UTSWeaponDataAsset* InWeaponData)
{
	WeaponData = InWeaponData;
	if (!WeaponData)
	{
		return;
	}

	CurrentAmmo = WeaponData->ClipSize;
	CurrentReserve = WeaponData->MaxReserveAmmo;
}

void ATSWeaponBase::OnRep_WeaponData()
{
	if (WeaponData && CurrentAmmo == 0 && CurrentReserve == 0)
	{
		CurrentAmmo = WeaponData->ClipSize;
		CurrentReserve = WeaponData->MaxReserveAmmo;
	}
}

bool ATSWeaponBase::CanFire() const
{
	return WeaponData && CurrentAmmo > 0;
}

void ATSWeaponBase::ConsumeAmmo()
{
	if (CurrentAmmo > 0)
	{
		--CurrentAmmo;
	}
}

int32 ATSWeaponBase::CalcReloadAmount() const
{
	if (!WeaponData)
	{
		return 0;
	}
	return FMath::Min(WeaponData->ClipSize - CurrentAmmo, CurrentReserve);
}

void ATSWeaponBase::ApplyReload()
{
	const int32 ReloadAmount = CalcReloadAmount();
	CurrentAmmo += ReloadAmount;
	CurrentReserve -= ReloadAmount;
}

FVector ATSWeaponBase::GetMuzzleLocation() const
{
	static const FName MuzzleSocketName(TEXT("Muzzle"));

	if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		return WeaponMesh->GetSocketLocation(MuzzleSocketName);
	}
	if (WeaponStaticMesh && WeaponStaticMesh->DoesSocketExist(MuzzleSocketName))
	{
		return WeaponStaticMesh->GetSocketLocation(MuzzleSocketName);
	}

	// 无 Muzzle socket 时退化为组件原点。
	if (WeaponMesh)
	{
		return WeaponMesh->GetComponentLocation();
	}
	return GetActorLocation();
}
