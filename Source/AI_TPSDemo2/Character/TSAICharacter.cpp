#include "Character/TSAICharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Core/TSAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

ATSAICharacter::ATSAICharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// AI 不绑定相机控制旋转，巡逻/移动时朝移动方向转身；攻击时面向目标由 AIController focus 处理（模块8）。
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 480.f, 0.f);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	}
}

void ATSAICharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void ATSAICharacter::OnDeath()
{
	Super::OnDeath();

	// 停止行为树逻辑，并在黑板上标记死亡（键不存在时无副作用）。
	if (AAIController* AICon = Cast<AAIController>(GetDeathController()))
	{
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
		if (UBlackboardComponent* Blackboard = AICon->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(TEXT("IsDead"), true);
		}
	}

	// 仅在 authority 侧安排清理定时器（决策点锁定：延时 5.0s 销毁）。
	if (HasAuthority())
	{
		FTimerHandle CleanupTimerHandle;
		GetWorldTimerManager().SetTimer(CleanupTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				Destroy();
			}),
			5.0f, false);
	}
}
