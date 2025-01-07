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

protected:
	virtual void BeginPlay() override;

	/* MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	/* Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Input_Jump;

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

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
	
public:
	
};
