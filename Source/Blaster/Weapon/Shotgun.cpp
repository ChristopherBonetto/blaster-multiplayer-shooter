// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (OwnerPawn == nullptr)
	{
		return;
	}

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		
		for (uint32 i = 0; i < NumberOfPellets; ++i)
		{
			FVector End = TraceEndWithScatter(Start, HitTarget);
		}
	}
}

