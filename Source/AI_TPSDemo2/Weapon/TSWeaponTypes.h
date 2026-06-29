// 武器槽位枚举（模块4）。

#pragma once

#include "CoreMinimal.h"
#include "TSWeaponTypes.generated.h"

UENUM(BlueprintType)
enum class ETSWeaponSlot : uint8
{
	Primary   UMETA(DisplayName = "Primary"),
	Secondary UMETA(DisplayName = "Secondary")
};
