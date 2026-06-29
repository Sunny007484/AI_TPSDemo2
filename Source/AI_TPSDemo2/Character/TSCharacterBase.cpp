#include "Character/TSCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Character/TSCharacterMovementComponent.h"
#include "Core/TSAbilitySystemComponent.h"
#include "Core/TSAttributeSet.h"
#include "Core/TSGameplayAbility.h"
#include "Core/TSGameplayTags.h"
#include "Core/TSWeaponAttributeSet.h"

ATSCharacterBase::ATSCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UTSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UTSAttributeSet>(TEXT("AttributeSet"));
	WeaponAttributeSet = CreateDefaultSubobject<UTSWeaponAttributeSet>(TEXT("WeaponAttributeSet"));
}

UAbilitySystemComponent* ATSCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ATSCharacterBase::InitAbilityActorInfo()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (HasAuthority())
	{
		ApplyDefaultEffects();
		GrantDefaultAbilities();
	}
}

void ATSCharacterBase::GrantDefaultAbilities()
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	TArray<TSubclassOf<UGameplayAbility>> Abilities;
	Abilities.Reserve(DefaultAbilities.Num());
	for (const TSubclassOf<UTSGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			Abilities.Add(AbilityClass);
		}
	}

	AbilitySystemComponent->GrantStartupAbilities(Abilities);
}

void ATSCharacterBase::ApplyDefaultEffects()
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		const FGameplayEffectSpecHandle SpecHandle =
			AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

bool ATSCharacterBase::IsDeadOrDying() const
{
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(TSGameplayTags::State_Dead);
}

void ATSCharacterBase::Jump()
{
	if (UTSCharacterMovementComponent* MoveComp = Cast<UTSCharacterMovementComponent>(GetCharacterMovement()))
	{
		if (MoveComp->IsSliding())
		{
			MoveComp->InterruptSlideForJump();
		}
	}

	Super::Jump();
}

void ATSCharacterBase::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(TSGameplayTags::State_Movement_Crouching);
	}
}

void ATSCharacterBase::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(TSGameplayTags::State_Movement_Crouching);
	}
}

void ATSCharacterBase::HandleDeath()
{
	// 模块6 实现：取消能力、Ragdoll、AI 停 BT、玩家延时重生。
}
