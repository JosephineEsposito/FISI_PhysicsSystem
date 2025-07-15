// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Weapons/WeaponDamageType.h"
#include "Weapons/PhysicsWeaponComponent.h"
#include <Kismet/GameplayStatics.h>

APhysicsProjectile::APhysicsProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &APhysicsProjectile::OnHit);

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void APhysicsProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Hit.GetComponent()->Mobility != EComponentMobility::Movable)
	{
		return;
	}

	float mass = OtherComp->CalculateMass();
	FVector acceleration = ProjectileMovement->ComputeAcceleration(ProjectileMovement->Velocity * NormalImpulse, GetWorld()->GetDeltaSeconds());
	FVector Force = acceleration * mass;

	switch (m_ProjectileType)
	{
	case EProjectileType::DEFAULT:
		OtherComp->AddImpulseAtLocation(Force, Hit.Location, FName("None"));
		break;
	case EProjectileType::GRANATE:
		OtherComp->AddRadialImpulse(GetActorLocation(), m_Radius, Force.Length(), ERadialImpulseFalloff::RIF_Linear, false);
		break;
	default:
		break;
	}
}
