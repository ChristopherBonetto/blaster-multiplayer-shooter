// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

	friend class ABlasterCharacter; //RENDE QUESTA CLASSE AMICA, DANDOLE ACCESSO A TUTTE LE VARIABILI E FUNZIONI DI QUESTA CLASSE. INCLUSE LE PRIVATE E PROTECTED!!!

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void EquipWeapon(AWeapon* WeaponToEquip);
	
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	//Solo mettere bAiming come variabile replicata non bastava, perchè funziona solo se stiamo giocando con il server (il server manda l'info ai client, ma i client non mostrano il cambio di stato agli altri)
	//Facciamo quindi una RPC per far gestire la chiamata al Server
	UFUNCTION(Server, Reliable) 
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

private:
	class ABlasterCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;
};
