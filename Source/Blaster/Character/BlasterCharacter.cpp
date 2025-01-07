// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

//EnhancedInput
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//Create and setup spring arm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	const APlayerController* PC = GetController<APlayerController>();
	const ULocalPlayer* LP = PC->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	Subsystem->ClearAllMappings();

	//Add mapping for our game, more complex games may have multiple Contexts that are added/removed at runtime
	Subsystem->AddMappingContext(DefaultInputMappingContext, 0);

	//New enhanced input system
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	//General
	EnhancedInputComponent->BindAction(Input_Move, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);
	EnhancedInputComponent->BindAction(Input_Jump, ETriggerEvent::Triggered, this, &ABlasterCharacter::Jump);
	EnhancedInputComponent->BindAction(Input_LookMouse, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookMouse);

	//Gamepad
	EnhancedInputComponent->BindAction(Input_LookStick, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookStick);
}

#pragma region Inputs
void ABlasterCharacter::LookMouse(const FInputActionValue& InputValue)
{
	const FVector2D Value = InputValue.Get<FVector2D>();
	
	AddControllerYawInput(Value.X);
	AddControllerPitchInput(Value.Y);
}

void ABlasterCharacter::LookStick(const FInputActionValue& InputValue)
{
	FVector2D Value = InputValue.Get<FVector2D>();

	// Track negative as we'll lose this during the conversion
	bool XNegative = Value.X < 0.f;
	bool YNegative = Value.Y < 0.f;

	// Can further modify with 'sensitivity' settings
	static const float LookYawRate = 100.0f;
	static const float LookPitchRate = 50.0f;

	// non-linear to make aiming a little easier
	Value = Value * Value;

	if (XNegative)
	{
		Value.X *= -1.f;
	}
	if (YNegative)
	{
		Value.Y *= -1.f;
	}

	// Aim assist
	// todo: may need to ease this out and/or change strength based on distance to target
	float RateMultiplier = 1.0f;
	// if (bHasPawnTarget)
	// {
	// 	RateMultiplier = 0.5f;
	// }

	AddControllerYawInput(Value.X * (LookYawRate * RateMultiplier) * GetWorld()->GetDeltaSeconds());
	AddControllerPitchInput(Value.Y * (LookPitchRate * RateMultiplier) * GetWorld()->GetDeltaSeconds());
}

void ABlasterCharacter::Move(const FInputActionInstance& Instance)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	// Get value from input (combined value from WASD keys or single Gamepad stick) and convert to Vector (x,y)
	const FVector2D AxisValue = Instance.GetValue().Get<FVector2D>();

	// Move forward/back
	AddMovementInput(ControlRot.Vector(), AxisValue.Y);

	// Move Right/Left strafe
	const FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	AddMovementInput(RightVector, AxisValue.X);

	// Alternative Copied from Lyra
	/*
	{
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}*/
}
#pragma endregion 

