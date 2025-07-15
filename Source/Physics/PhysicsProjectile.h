// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPhysicsWeaponComponent;

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	DEFAULT UMETA(DisplayName = "Default"),
	GRANATE UMETA(DisplayName = "Granate")
};


UCLASS(config=Game)
class APhysicsProjectile : public AActor
{
	GENERATED_BODY()

public:
	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	/** Destroy projectile on Hit*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool m_DestroyOnHit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Damage)
	UPhysicsWeaponComponent* m_OwnerWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EProjectileType m_ProjectileType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float m_Radius;

public:
	APhysicsProjectile();

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};

