// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "WeaponTypes.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

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
		FVector End = Start + (HitTarget - Start) * 1.25f; //From start location, we summ the vector from the start to the hit target extended by a 25% 

		FHitResult FireHit;
		UWorld* World = GetWorld();

		if (World)
		{
			FVector BeamEnd = End;
			
			World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);
			
			if (FireHit.bBlockingHit)
			{
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
				AController* InstigatorController = OwnerPawn->GetController();
				BeamEnd = FireHit.ImpactPoint;

				if (BlasterCharacter && HasAuthority() && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
				}
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactNormal, FireHit.ImpactNormal.Rotation());
				}
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
				}
			}
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlash, SocketTransform);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	FVector RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	
	FVector EndLocation = SphereCenter + RandVect;
	
	FVector ToEndLoc = EndLocation - TraceStart;
	
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(GetWorld(), TraceStart, TraceStart + ToEndLoc * TRACE_LENGHT / ToEndLoc.Size(), FColor::Cyan, true);
	
	return FVector(TraceStart + ToEndLoc * TRACE_LENGHT / ToEndLoc.Size());
}
