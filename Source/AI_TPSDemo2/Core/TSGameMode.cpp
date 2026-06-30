// 游戏模式实现（模块9）。

#include "Core/TSGameMode.h"

#include "Core/TSPlayerController.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

ATSGameMode::ATSGameMode()
{
	PlayerControllerClass = ATSPlayerController::StaticClass();

	// 默认玩家角色：查找蓝图 BP_TSPlayerCharacter（找不到则保留引擎默认，不报错）。
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnFinder(
		TEXT("/Game/TPS/Blueprints/BP_TSPlayerCharacter.BP_TSPlayerCharacter_C"));
	if (PlayerPawnFinder.Succeeded())
	{
		DefaultPawnClass = PlayerPawnFinder.Class;
	}
}
