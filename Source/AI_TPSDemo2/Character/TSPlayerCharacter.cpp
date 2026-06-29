#include "Character/TSPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Core/TSAbilitySystemComponent.h"
#include "Core/TSInputConfig.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

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
}

void ATSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void ATSPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
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
