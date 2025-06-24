// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(Meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* ScoreAmount;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* DefeatsAmount;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* DefeatMessageText;
};
