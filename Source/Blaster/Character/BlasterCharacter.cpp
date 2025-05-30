// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

//EnhancedInput
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blaster/Blaster.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/GameMode/BlasterGameMode.h"

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

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true); //LO RENDE REPLICATO, I COMPONENTI SONO SPECIALI E NON HANNO BISOGNO DI ESSERE REGISTRATI IN "GetLifetimeReplicatedProps()"

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	GetCharacterMovement()->RotationRate = FRotator(0.f,0.f,850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon); --> Questo non va bene perchè registra questa variabile come replicata per tutti, quindi cambia a tutti i client e non solo a chi si è avvicinato all'arma
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly); //Questo è giusto, aggiunge la condizione di essere l'owner cioè il client che sta giocando
	DOREPLIFETIME(ABlasterCharacter, Health);
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled()) //TO AVOID TO CALL THIS FUNCTION FOR SIMULATED PROXY
	// {
	// 	AimOffset(DeltaTime);
	// }
	// else
	// {
	// 	TimeSinceLastMovementReplication += DeltaTime;
	// 	if (TimeSinceLastMovementReplication > 0.25f)
	// 	{
	// 		OnRep_ReplicatedMovement();
	// 	}
	// 	CalculateAO_Pitch();
	// }
	AimOffset(DeltaTime);
	
	HideCameraIfCharacterClose();
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this; //CHARACTER è PRIVATA, MA SICCOME ABBIAMO RESO LA CLASSE DI QUESTO COMPONENTE AMICA A BlasterCharacter ABBIAMO ACCESSO A TUTTO DA QUESTA CLASSE
	}
}

void ABlasterCharacter::Elim()
{
	//Called just on the Server
	
	MulticastElim();
	
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();

	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		
		FName SectionName;
		SectionName = bAiming? FName("RifleAim") : FName( "RifleHip");

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		
		FName SectionName("FromFront");

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser)
{
	//This method is called just on the server, it changes the health value and so it will call OnRep_Health for all the client.
	//It has to play the ReactMontage here too, because the player who is the server needs to see its react animation too 
	
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	UpdateHUDHealth();
	
	PlayHitReactMontage();

	if (Health <= 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		
		}
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr? Cast<ABlasterPlayerController>(Controller): BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	
	OverlappingWeapon = Weapon;

	if (IsLocallyControlled()) //QUANDO SETTIAMO LA VARIABILE, SE SIAMO IL SERVER, MOSTRIAMO IL WIDGET, perchè il server non riceve OnRepNotifies
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr)
	{
		return nullptr;
	}
	return Combat->EquippedWeapon;
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr)
	{
		return FVector();
	}
	return Combat->HitTarget;
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon) //CHIAMATO SOLO NEI CLIENTS
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
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
	EnhancedInputComponent->BindAction(Input_Crouch, ETriggerEvent::Started, this, &ABlasterCharacter::CrouchButtonPressed);
	
	EnhancedInputComponent->BindAction(Input_LookMouse, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookMouse);
	
	EnhancedInputComponent->BindAction(Input_Aim, ETriggerEvent::Started, this, &ABlasterCharacter::AimButtonPressed);
	EnhancedInputComponent->BindAction(Input_Aim, ETriggerEvent::Completed, this, &ABlasterCharacter::AimButtonReleased);

	EnhancedInputComponent->BindAction(Input_Fire, ETriggerEvent::Started, this, &ABlasterCharacter::FireButtonPressed);
	EnhancedInputComponent->BindAction(Input_Fire, ETriggerEvent::Completed, this, &ABlasterCharacter::FireButtonReleased);

	EnhancedInputComponent->BindAction(Input_Equip, ETriggerEvent::Triggered, this, &ABlasterCharacter::EquipButtonPressed);

	//Gamepad
	EnhancedInputComponent->BindAction(Input_LookStick, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookStick);
}

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

void ABlasterCharacter::EquipButtonPressed()
{
	if (Combat) 
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon); //Se ha l'autorità equipaggia l'arma, se no, manda una RPC/richiesta al server e sarà lui a mandare la risposta
		}
		else
		{
			ServerEquipButtonPressed(); //Non server usare il nome ServerEquipButtonPressed_Implementation perche _Implementation serve solo per la dicitura
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::CrouchButtonPressed()
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

void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

float ABlasterCharacter::CalculatedSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr)
	{
		return;
	}
	
	float Speed = CalculatedSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) //standing still, not jumping
	{
		#pragma region sync proxies turn animation - deprecated
		//NOT USED BECAUSE SOLVED USING LYRA SOLUTION, setting Linear on the setting "Network Smoothing Mode" in BP_Blaster blueprint
		//bRotateRootBone = true;
		#pragma endregion
		
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation); //la differenza tra la rotazione corrente e quella iniziale
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) //running or jumping
	{
		#pragma region sync proxies turn animation - deprecated
		//NOT USED BECAUSE SOLVED USING LYRA SOLUTION, setting Linear on the setting "Network Smoothing Mode" in BP_Blaster blueprint
		//bRotateRootBone = true;
		#pragma endregion
		
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	//sui client il valore di rotazione non rimane compreso tra -90 e 0, ma a volte prende valori molto alti solo quando si guarda in basso. Questo è dovuto perchè quando
	//vengono mandati i pacchetti di dati al server e ai client vengono trattati senza segno e convertiti quindi usando valori tra 0 e 360. Quindi qua mappiamo quel valore per i giocatori del client per renderlo di nuovo
	//compreso tra -90 e 0
	if (AO_Pitch > 90.f && !IsLocallyControlled()) 
	{
		// map pitch from the range [270, 360) to [-90, 0) --> parentesi quadra vuol dire che è incluso, tonda vuol dire che non lo è
		FVector2d InRange(270.f, 360.f);
		FVector2d OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;

		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

#pragma region sync proxies turn animation - deprecated
//NOT USED BECAUSE SOLVED USING LYRA SOLUTION, setting Linear on the setting "Network Smoothing Mode" in BP_Blaster blueprint
// void ABlasterCharacter::OnRep_ReplicatedMovement()
// {
// 	Super::OnRep_ReplicatedMovement();
//
// 	SimProxiesTurn();
// 	TimeSinceLastMovementReplication = 0.f;
// }

// void ABlasterCharacter::SimProxiesTurn()
// {
// 	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
// 	{
// 		return;
// 	}
//
// 	bRotateRootBone = false;
// 	float Speed = CalculatedSpeed();
//
// 	if (Speed > 0.f)
// 	{
// 		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
// 		return;
// 	}
// 	
// 	ProxyRotationLastFrame = ProxyRotation;
// 	ProxyRotation = GetActorRotation();
// 	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
//
// 	UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);
// 	
// 	if (FMath::Abs(ProxyYaw) > TurnThreshold)
// 	{
// 		bUseControllerRotationYaw = true ;
// 		if (ProxyYaw > TurnThreshold)
// 		{
// 			TurningInPlace = ETurningInPlace::ETIP_Right;
// 		}
// 		else if (ProxyYaw < -TurnThreshold)
// 		{
// 			TurningInPlace = ETurningInPlace::ETIP_Left;
// 		}
// 		else
// 		{
// 			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
// 		}
// 		return;
// 	}
// 	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
// }
#pragma endregion sync proxies turn animation

