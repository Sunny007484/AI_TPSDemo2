// 游戏模式：指定 TS 玩家控制器与默认玩家角色蓝图（模块9）。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TSGameMode.generated.h"

UCLASS()
class AI_TPSDEMO2_API ATSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATSGameMode();
};
