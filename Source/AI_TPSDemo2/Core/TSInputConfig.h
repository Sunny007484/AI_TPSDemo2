// 输入配置数据资产：维护 InputAction → Ability InputTag 的映射表（模块10a）。

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "TSInputConfig.generated.h"

class UInputAction;

USTRUCT(BlueprintType)
struct FTSInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "Ability"))
	FGameplayTag InputTag;
};

UCLASS()
class AI_TPSDEMO2_API UTSInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// 战斗/移动类输入：按下/松开会被路由到 ASC 的 AbilityInputTagPressed/Released。
	UPROPERTY(EditDefaultsOnly, Category = "TS|Input")
	TArray<FTSInputAction> AbilityInputActions;
};
