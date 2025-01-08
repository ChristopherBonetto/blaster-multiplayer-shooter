// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class UInputMappingContext;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;

	/* MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

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
	
public:
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();
};
