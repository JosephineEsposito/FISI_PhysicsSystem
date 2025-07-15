// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/HitscanWeaponComponent.h"
#include <Kismet/KismetSystemLibrary.h>
#include "Kismet/GameplayStatics.h"
#include "PhysicsCharacter.h"
#include "PhysicsWeaponComponent.h"
#include <Camera/CameraComponent.h>
#include <Components/SphereComponent.h>

void UHitscanWeaponComponent::Fire()
{
	Super::Fire();

	if (!IsValid(GetOwner()))
	{
		return;
	}

	FVector lStart = GetComponentLocation();
	FVector lForward = Character->FirstPersonCameraComponent->GetForwardVector();
	FVector lEnd = lStart + (lForward * m_fRange);

	FVector acceleration = lForward * m_fAcceleration;
	FVector Force = acceleration * m_fMass;

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	if (GetWorld()->LineTraceSingleByChannel(Hit, lStart, lEnd, ECC_Visibility, QueryParams))
	{
		AActor* other = Hit.GetActor();
		ApplyDamage(other, Hit);
		onHitscanImpact.Broadcast(other, Hit.ImpactPoint, lForward);

		UPrimitiveComponent* HitComp = Cast<UPrimitiveComponent>(Hit.GetComponent());
		if (!IsValid(HitComp))
		{
			return;
		}

		if (HitComp->Mobility == EComponentMobility::Movable)
		{
			HitComp->AddImpulseAtLocation(Force, Hit.Location, FName("None"));
		}
	}
}
//EOF