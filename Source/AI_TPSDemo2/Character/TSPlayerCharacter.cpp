#include "Character/TSPlayerCharacter.h"

#include "Abilities/TSGA_ADS.h"
#include "Abilities/TSGA_Fire.h"
#include "Abilities/TSGA_Reload.h"
#include "Abilities/TSGA_WeaponSwitch.h"
#include "Abilities/TSGE_InitAttributes.h"
#include "Camera/CameraComponent.h"
#include "Core/TSAbilitySystemComponent.h"
#include "Core/TSGameplayTags.h"
#include "Core/TSInputConfig.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"
#include "Weapon/TSCombatComponent.h"
#include "Weapon/TSWeaponBase.h"
#include "Weapon/TSWeaponDataAsset.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

ATSPlayerCharacter::ATSPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// COD 越肩 strafe：角色始终面向相机朝向，不自动转向移动方向。
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = false;
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.f;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 60.f);
	CameraBoom->bUsePawnControlRotation = true;

	PrimaryActorTick.bCanEverTick = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 默认复用第三人称模板输入资产（蓝图可覆盖）。
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCDefault(
		TEXT("/Game/Input/IMC_Default.IMC_Default"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCMouseLook(
		TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	static ConstructorHelpers::FObjectFinder<UInputAction> IAMove(
		TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction> IALook(
		TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	static ConstructorHelpers::FObjectFinder<UInputAction> IAMouseLook(
		TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));
	static ConstructorHelpers::FObjectFinder<UInputAction> IAJump(
		TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));

	if (IMCDefault.Succeeded()) { DefaultMappingContext = IMCDefault.Object; }
	if (IMCMouseLook.Succeeded()) { MouseLookMappingContext = IMCMouseLook.Object; }
	if (IAMove.Succeeded()) { MoveAction = IAMove.Object; }
	if (IALook.Succeeded()) { LookAction = IALook.Object; }
	if (IAMouseLook.Succeeded()) { MouseLookAction = IAMouseLook.Object; }
	if (IAJump.Succeeded()) { JumpAction = IAJump.Object; }

	// 战斗能力 C++ 默认注册（蓝图可追加 Sprint/Slide 等）。
	DefaultAbilities.Add(UTSGA_Fire::StaticClass());
	DefaultAbilities.Add(UTSGA_ADS::StaticClass());
	DefaultAbilities.Add(UTSGA_Reload::StaticClass());
	DefaultAbilities.Add(UTSGA_WeaponSwitch::StaticClass());
	DefaultEffects.Add(UTSGE_InitAttributes::StaticClass());
}

void ATSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void ATSPlayerCharacter::GrantDefaultAbilities()
{
	// 确保战斗能力始终授予（蓝图 DefaultAbilities 可能只含 Sprint/Slide）。
	DefaultAbilities.AddUnique(UTSGA_Fire::StaticClass());
	DefaultAbilities.AddUnique(UTSGA_ADS::StaticClass());
	DefaultAbilities.AddUnique(UTSGA_Reload::StaticClass());
	DefaultAbilities.AddUnique(UTSGA_WeaponSwitch::StaticClass());
	DefaultEffects.AddUnique(UTSGE_InitAttributes::StaticClass());

	Super::GrantDefaultAbilities();
}

void ATSPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

void ATSPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bShowCombatDebug || !IsLocallyControlled())
	{
		return;
	}

	UTSCombatComponent* Combat = GetCombatComponent();
	if (!Combat)
	{
		return;
	}

	FString DebugText = TEXT("Combat Debug\n");
	if (const ATSWeaponBase* Current = Combat->GetCurrentWeapon())
	{
		DebugText += FString::Printf(TEXT("Current Slot: %d | %s\nAmmo: %d / Reserve: %d\n"),
			Combat->GetCurrentSlotIndex(),
			Current->GetWeaponData() ? *Current->GetWeaponData()->WeaponName.ToString() : TEXT("Unknown"),
			Current->GetCurrentAmmo(), Current->GetCurrentReserve());

		const FVector Muzzle = Current->GetMuzzleLocation();
		DrawDebugSphere(GetWorld(), Muzzle, 6.f, 8, FColor::Orange, false, 0.f, 0, 1.5f);
		DebugText += FString::Printf(TEXT("Muzzle: %s"), *Muzzle.ToCompactString());
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.f, FColor::Cyan, DebugText);
	}
}

void ATSPlayerCharacter::OnDeath()
{
	Super::OnDeath();

	// 取死亡时缓存的 PlayerController 并禁用输入。
	if (APlayerController* PC = Cast<APlayerController>(GetDeathController()))
	{
		DisableInput(PC);
	}

	// 仅在 authority 侧安排重生定时器（决策点锁定：延时 3.0s 自动重生）。
	if (HasAuthority())
	{
		FTimerHandle RespawnTimerHandle;
		GetWorldTimerManager().SetTimer(RespawnTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				// 缓存控制器，因为 UnPossess 后 GetController() 失效。
				AController* CachedController = GetDeathController();
				if (UWorld* World = GetWorld())
				{
					if (AGameModeBase* GM = World->GetAuthGameMode())
					{
						if (CachedController)
						{
							CachedController->UnPossess();
							GM->RestartPlayer(CachedController);
						}
					}
				}
				// 销毁当前 ragdoll Pawn。
				Destroy();
			}),
			3.0f, false);
	}
}

void ATSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
			if (MouseLookMappingContext)
			{
				Subsystem->AddMappingContext(MouseLookMappingContext, 1);
			}
		}
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATSPlayerCharacter::Input_Move);
	}
	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATSPlayerCharacter::Input_Look);
	}
	if (MouseLookAction)
	{
		EIC->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATSPlayerCharacter::Input_Look);
	}
	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (CrouchAction)
	{
		EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATSPlayerCharacter::Input_Crouch);
	}

	if (InputConfig)
	{
		for (const FTSInputAction& Action : InputConfig->AbilityInputActions)
		{
			if (!Action.InputAction || !Action.InputTag.IsValid())
			{
				continue;
			}

			// 切枪输入单独处理，以便写入目标槽位。
			const FString ActionName = Action.InputAction->GetName();
			if (ActionName.Contains(TEXT("WeaponSlot1")))
			{
				EIC->BindAction(Action.InputAction, ETriggerEvent::Started, this,
					&ATSPlayerCharacter::RequestWeaponSwitch, 0);
				continue;
			}
			if (ActionName.Contains(TEXT("WeaponSlot2")))
			{
				EIC->BindAction(Action.InputAction, ETriggerEvent::Started, this,
					&ATSPlayerCharacter::RequestWeaponSwitch, 1);
				continue;
			}
			if (ActionName.Contains(TEXT("WeaponScroll")))
			{
				EIC->BindAction(Action.InputAction, ETriggerEvent::Started, this,
					&ATSPlayerCharacter::RequestWeaponSwitch, -2);
				continue;
			}

			if (Action.TargetWeaponSlot != INDEX_NONE)
			{
				const int32 Slot = Action.TargetWeaponSlot;
				EIC->BindAction(Action.InputAction, ETriggerEvent::Started, this,
					&ATSPlayerCharacter::RequestWeaponSwitch, Slot);
				continue;
			}

			EIC->BindAction(Action.InputAction, ETriggerEvent::Started, this,
				&ATSPlayerCharacter::Input_AbilityTagPressed, Action.InputTag);
			EIC->BindAction(Action.InputAction, ETriggerEvent::Completed, this,
				&ATSPlayerCharacter::Input_AbilityTagReleased, Action.InputTag);
		}
	}
}

void ATSPlayerCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	const AController* PC = GetController();
	if (!PC)
	{
		return;
	}

	const FRotator YawRotation(0.f, PC->GetControlRotation().Yaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, Axis.Y);
	AddMovementInput(RightDir, Axis.X);
}

void ATSPlayerCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ATSPlayerCharacter::Input_Crouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ATSPlayerCharacter::Input_AbilityTagPressed(FGameplayTag InputTag)
{
	if (UTSAbilitySystemComponent* ASC = GetTSAbilitySystemComponent())
	{
		ASC->AbilityInputTagPressed(InputTag);
	}
}

void ATSPlayerCharacter::Input_AbilityTagReleased(FGameplayTag InputTag)
{
	if (UTSAbilitySystemComponent* ASC = GetTSAbilitySystemComponent())
	{
		ASC->AbilityInputTagReleased(InputTag);
	}
}

void ATSPlayerCharacter::RequestWeaponSwitch(int32 SlotIndex)
{
	if (UTSCombatComponent* Combat = GetCombatComponent())
	{
		Combat->SetPendingSwitchSlot(SlotIndex);
	}
	Input_AbilityTagPressed(TSGameplayTags::Ability_WeaponSwitch);
}
