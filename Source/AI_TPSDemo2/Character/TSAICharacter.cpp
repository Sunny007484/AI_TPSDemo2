#include "Character/TSAICharacter.h"

#include "Core/TSAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
