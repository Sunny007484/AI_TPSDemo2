// AI 角色：与玩家共享战斗载体，朝移动方向转身，由 ATSAIController 接管（模块8）。

#pragma once

#include "CoreMinimal.h"
#include "Character/TSCharacterBase.h"
#include "TSAICharacter.generated.h"

UCLASS()
class AI_TPSDEMO2_API ATSAICharacter : public ATSCharacterBase
{
	GENERATED_BODY()

public:
	ATSAICharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;
};
