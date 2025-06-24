// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

class UInputMappingContext;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();

	void Elim();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;

	/* MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	/* Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Input_Fire;
	
	/* Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Input_Jump;

	/* Equip Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Input_Equip;

	/* Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Input_Crouch;

	/* Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Input_Move;

	/* Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Input_Aim;
	
	/* Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Input_LookMouse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Input_LookStick;
	
	void Move(const FInputActionInstance& Instance);

	void LookMouse(const FInputActionValue& InputValue);
	
	void LookStick(const FInputActionValue& InputValue);

	void EquipButtonPressed();

	void CrouchButtonPressed();

	void AimButtonPressed();
	void AimButtonReleased();
	void CalculateAO_Pitch();

	void AimOffset(float DeltaTime);
	
	virtual void Jump() override;

	void FireButtonPressed();
	void FireButtonReleased();

	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void UpdateHUDHealth();

	void ShowHUDDefeatMessage(bool IsVisible);

	// Poll for any relevant classes and init our HUD
	void PollInit();
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	//Abbiamo aggiunto questo OnRepNotify per sapere quando un character deve mostrare il messaggio che è vicino a qualcosa da prendere,
	//senza questo modo veniva mostrato il messaggio sopra l'arma del giocatore CLIENT e anche nel SERVER. Siccome OnRepNotify è CHIAMATO SOLO NEI CLIENTS adesso il messaggio lo vede solo più il client interessato
	//Ma cosi facendo non è possibile vederlo nel giocatore che sta usando il SERVER, per questo in "SetOverlappingWeapon()" controlliamo se siamo il server
	//NON VIENE CHIAMATA DA NOI, QUANDO CAMBIA LA VARIABILE ASSOCIATA CHIAMA QUESTO METODO. PUO' AVERE SOLO UNA VARIABILE IN INPUT DEL TIPO DELLA VAR CHE STA REPLICANDO
	//LastWeapon contiene il valore che aveva in precedenza
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable) //RPC, puo essere reliable (lenta ma sicura) o non-reliable (piu veloce ma meno sicura, non assicura che l'informazione/pacchetto dati arrivi)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ElimMontage;
		
	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	float CalculatedSpeed();
	
	/**
	* Player Health 
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	
	void ElimTimerFinished();

	/**
	* Dissolve Effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	//Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	//Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/*
	 * Elim bot
	*/

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
	
public:
	void SetOverlappingWeapon(AWeapon* Weapon);

	AWeapon* GetEquippedWeapon();
	bool IsWeaponEquipped();

	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health;}
	FORCEINLINE float GetMaxHealth() const { return MaxHealth;}

	FVector GetHitTarget() const;

#pragma region sync proxies turn animation - deprecated
	//NOT USED BECAUSE SOLVED USING LYRA SOLUTION, setting Linear on the setting "Network Smoothing Mode" in BP_Blaster blueprint
private:
	//NOT USED BECAUSE SOLVED USING LYRA SOLUTION, setting Linear on the setting "Network Smoothing Mode" in BP_Blaster blueprint
	//bool bRotateRootBone;
	// float TurnThreshold = 0.5f;
	// float ProxyYaw;
	// float TimeSinceLastMovementReplication;
	// FRotator ProxyRotationLastFrame;
	// FRotator ProxyRotation;
	
protected:
	//void SimProxiesTurn();

public:
	//virtual void OnRep_ReplicatedMovement() override;
	//FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	
#pragma endregion
	
};

inline void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}
