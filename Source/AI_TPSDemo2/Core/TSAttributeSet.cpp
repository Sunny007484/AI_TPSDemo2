#include "Core/TSAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/TSCharacterBase.h"
#include "Core/TSGameplayTags.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UTSAttributeSet::UTSAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitDamage(0.f);
}

void UTSAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTSAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTSAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UTSAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
}

void UTSAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 把瞬时 Damage meta 属性结算进 Health（模块1 已有 clamp 逻辑保留），
	// 模块6 在此基础上叠加伤害事件、击杀归属与死亡触发。
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// 取出本次伤害值并立即清零 meta 通道。
		const float LocalDamage = GetDamage();
		SetDamage(0.f);

		// 仅在确有伤害时结算并广播事件。
		if (LocalDamage > 0.f)
		{
			const float NewHealth = FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth());
			SetHealth(NewHealth);

			// 从效果上下文解析伤害来源（来源 Actor / 来源 ASC）。
			const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
			AActor* SourceActor = Context.GetEffectCauser();
			if (!SourceActor)
			{
				SourceActor = Context.GetOriginalInstigator();
			}
			if (!SourceActor)
			{
				SourceActor = Context.GetInstigator();
			}

			// 受击者 Actor：优先用 AttributeSet 拥有者，回退到 Target avatar。
			AActor* TargetActor = GetOwningActor();
			if (!TargetActor)
			{
				TargetActor = Data.Target.GetAvatarActor();
			}

			if (TargetActor)
			{
				// 向受击者发送 TakeDamage 事件；Instigator 必须为来源角色，
				// 供模块7 用其位置计算受击方向。
				FGameplayEventData DamagePayload;
				DamagePayload.Instigator = SourceActor;
				DamagePayload.Target = TargetActor;
				DamagePayload.EventMagnitude = LocalDamage;
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
					TargetActor, TSGameplayTags::Event_Combat_TakeDamage, DamagePayload);

				// 致命判定：血量归零且受击者尚未进入死亡流程。
				ATSCharacterBase* TargetCharacter = Cast<ATSCharacterBase>(TargetActor);
				if (GetHealth() <= 0.f && TargetCharacter && !TargetCharacter->IsDeadOrDying())
				{
					// 先向来源发送击杀事件（击杀归属），再触发受击者死亡处理。
					if (SourceActor)
					{
						// 击杀事件须发给持有 ASC 的击杀者 Pawn；来源解析为 Controller 时回退到其 Pawn，
						// 否则 PlayerController（无 ASC）会导致事件派发失败，命中/击杀反馈无法触发。
						AActor* KillEventActor = SourceActor;
						if (AController* SourceController = Cast<AController>(KillEventActor))
						{
							if (APawn* SourcePawn = SourceController->GetPawn())
							{
								KillEventActor = SourcePawn;
							}
						}

						FGameplayEventData KillPayload;
						KillPayload.Instigator = SourceActor;
						KillPayload.Target = TargetActor;
						KillPayload.EventMagnitude = LocalDamage;
						UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
							KillEventActor, TSGameplayTags::Event_Combat_Kill, KillPayload);
					}

					TargetCharacter->HandleDeath();
				}
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

void UTSAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTSAttributeSet, Health, OldHealth);
}

void UTSAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTSAttributeSet, MaxHealth, OldMaxHealth);
}
