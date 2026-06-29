// 伤害 GameplayEffect：SetByCaller Data.Damage → Damage meta 属性（模块5）。

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "TSGE_Damage.generated.h"

UCLASS()
class AI_TPSDEMO2_API UTSGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UTSGE_Damage();
};
