#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	
	EWT_MAX UMETA(DisplayName = "Default MAX") //can return the numeric max of types contained into this enum 
};