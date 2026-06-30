// 玩家控制器实现（模块9 创建 HUD / 重绑；模块7 追加战斗事件订阅与命中反馈）。

#include "Core/TSPlayerController.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Blueprint/UserWidget.h"
#include "Character/TSCharacterBase.h"
#include "Core/TSGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UI/TSUserWidget.h"

ATSPlayerController::ATSPlayerController()
{
}

void ATSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 仅本地控制器创建 HUD，且需配置控件类。
	if (IsLocalController() && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UTSUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			HUDWidget->InitializeForPlayer(this);
		}
	}
}

void ATSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 结束时确保解绑，避免悬垂委托。
	UnsubscribeCombatEvents();

	Super::EndPlay(EndPlayReason);
}

void ATSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 重生换 pawn 后让 HUD 重新解析并绑定到新 pawn。
	if (HUDWidget)
	{
		HUDWidget->RebindToOwner();
	}

	// 模块7：订阅新 pawn ASC 的战斗事件（命中/击杀/受伤）。
	if (ATSCharacterBase* TS = Cast<ATSCharacterBase>(InPawn))
	{
		if (UAbilitySystemComponent* PawnASC = TS->GetAbilitySystemComponent())
		{
			SubscribeCombatEvents(PawnASC);
		}
	}
}

void ATSPlayerController::OnUnPossess()
{
	// 失去 pawn 前先解绑战斗事件。
	UnsubscribeCombatEvents();

	Super::OnUnPossess();
}

void ATSPlayerController::SubscribeCombatEvents(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		return;
	}

	// 先清理旧订阅，避免重复注册或悬垂句柄。
	UnsubscribeCombatEvents();

	HitEventHandle = InASC->GenericGameplayEventCallbacks.FindOrAdd(TSGameplayTags::Event_Combat_Hit)
		.AddUObject(this, &ATSPlayerController::HandleHitEvent);
	KillEventHandle = InASC->GenericGameplayEventCallbacks.FindOrAdd(TSGameplayTags::Event_Combat_Kill)
		.AddUObject(this, &ATSPlayerController::HandleKillEvent);
	TakeDamageEventHandle = InASC->GenericGameplayEventCallbacks.FindOrAdd(TSGameplayTags::Event_Combat_TakeDamage)
		.AddUObject(this, &ATSPlayerController::HandleTakeDamageEvent);

	SubscribedASC = InASC;
}

void ATSPlayerController::UnsubscribeCombatEvents()
{
	if (UAbilitySystemComponent* ASC = SubscribedASC.Get())
	{
		ASC->GenericGameplayEventCallbacks.FindOrAdd(TSGameplayTags::Event_Combat_Hit).Remove(HitEventHandle);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(TSGameplayTags::Event_Combat_Kill).Remove(KillEventHandle);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(TSGameplayTags::Event_Combat_TakeDamage).Remove(TakeDamageEventHandle);
	}

	HitEventHandle.Reset();
	KillEventHandle.Reset();
	TakeDamageEventHandle.Reset();
	SubscribedASC.Reset();
}

void ATSPlayerController::HandleHitEvent(const FGameplayEventData* Payload)
{
	// 普通命中：触发命中标记（非击杀）。
	if (HUDWidget)
	{
		HUDWidget->OnHitMarker(false);
	}
}

void ATSPlayerController::HandleKillEvent(const FGameplayEventData* Payload)
{
	// 击杀确认：触发击杀样式命中标记，并播放击杀音效（若配置）。
	if (HUDWidget)
	{
		HUDWidget->OnHitMarker(true);
	}

	if (KillSound)
	{
		UGameplayStatics::PlaySound2D(this, KillSound);
	}
}

void ATSPlayerController::HandleTakeDamageEvent(const FGameplayEventData* Payload)
{
	if (!HUDWidget)
	{
		return;
	}

	// 伤害来源 Actor（Instigator 为伤害来源角色）。
	const AActor* Source = Payload ? Payload->Instigator.Get() : nullptr;
	const APawn* PlayerPawn = GetPawn();
	if (!Source || !PlayerPawn)
	{
		return;
	}

	// 玩家前向用控制朝向 yaw（仅水平面）。
	const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);

	// 指向伤害来源的水平方向。
	FVector ToSource = Source->GetActorLocation() - PlayerPawn->GetActorLocation();
	ToSource.Z = 0.f;
	if (!ToSource.Normalize())
	{
		// 来源与玩家重合，方向无意义，按正前处理。
		HUDWidget->OnDamageDirection(0.f);
		return;
	}

	// 叉积 Z 决定左右（正=右，负=左），点积决定前后；Atan2 得到 -180~180 度。
	const float CrossZ = FVector::CrossProduct(Forward, ToSource).Z;
	const float Dot = FVector::DotProduct(Forward, ToSource);
	const float Angle = FMath::RadiansToDegrees(FMath::Atan2(CrossZ, Dot));

	HUDWidget->OnDamageDirection(Angle);
}
