// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsCharacter.h"
#include "PhysicsProjectile.h"

#include "Animation/AnimInstance.h"

#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Engine/LocalPlayer.h"

#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "PhysicsEngine/PhysicsHandleComponent.h"

#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

APhysicsCharacter::APhysicsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	m_PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}

void APhysicsCharacter::BeginPlay()
{
	Super::BeginPlay();

	uCharacterComponent = GetCharacterMovement();

	m_SprintSpeedMultiplier = 1.5f;
	m_MaxWalkSpeed = uCharacterComponent->MaxWalkSpeed;
	m_CurrentStamina = m_MaxStamina;
}

void APhysicsCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


	// @TODO: Stamina update

	if (!m_bIsRunning && m_CurrentStamina < m_MaxStamina)
	{
		m_CurrentStamina += 1.0f;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Stamina is at <%.2f>."), m_CurrentStamina);


	FHitResult Hit;
	FVector lForward = FirstPersonCameraComponent->GetForwardVector();
	FVector lStart = GetActorLocation() + FirstPersonCameraComponent->GetRelativeLocation();
	FVector lEnd = lStart + lForward * m_CurrentGrabDistance;

	// @TODO: Physics objects highlight
	if (GetWorld()->LineTraceSingleByChannel(Hit, lStart, lStart + lForward * m_MaxGrabDistance, ECC_Visibility))
	{
		if (Hit.GetComponent()->Mobility == EComponentMobility::Movable)
		{ // only highlight if movable
			m_uGrabbedMesh = Cast<UMeshComponent>(Hit.GetComponent());
			if (IsValid(m_uGrabbedMesh))
			{
				SetHighlightedMesh(m_uGrabbedMesh);
			}
		}
	}

	// @TODO: Grabbed object update
	if (m_bIsGrabbing)
	{
		m_PhysicsHandle->SetTargetLocation(lEnd);
	}
}

void APhysicsCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APhysicsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Look);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APhysicsCharacter::Sprint);
		EnhancedInputComponent->BindAction(PickUpAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::GrabObject);
		EnhancedInputComponent->BindAction(PickUpAction, ETriggerEvent::Completed, this, &APhysicsCharacter::ReleaseObject);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APhysicsCharacter::SetIsSprinting(bool NewIsSprinting)
{
	// @TODO: Enable/disable sprinting use CharacterMovementComponent
	if (NewIsSprinting)
	{
		// can sprint
		uCharacterComponent->MaxWalkSpeed = m_MaxWalkSpeed * m_SprintSpeedMultiplier;
		if (m_CurrentStamina < 0)
		{
			m_CurrentStamina = 0;
		}
		else
		{
			m_CurrentStamina -= 1.0f;
		}
	}
	else
	{
		// cannot sprint
		uCharacterComponent->MaxWalkSpeed = m_MaxWalkSpeed;
	}

	m_bIsRunning = NewIsSprinting;
}

void APhysicsCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APhysicsCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APhysicsCharacter::Sprint(const FInputActionValue& Value)
{
	if (m_CurrentStamina > 1)
	{
		SetIsSprinting(Value.Get<bool>());
	}
}

void APhysicsCharacter::GrabObject(const FInputActionValue& Value)
{
	// @TODO: Grab objects using UPhysicsHandleComponent
	FHitResult Hit;
	FVector lForward = FirstPersonCameraComponent->GetForwardVector();
	FVector lStart = GetActorLocation() + FirstPersonCameraComponent->GetRelativeLocation();
	FVector lEnd = lStart + lForward * m_MaxGrabDistance;

	if (GetWorld()->LineTraceSingleByChannel(Hit, lStart, lEnd, ECC_Visibility))
	{
		if (!Hit.GetComponent() && !m_bIsGrabbing)
		{
			//UE_LOG(LogTemp, Error, TEXT("Nope"));
			return;
		}
		if (Hit.GetComponent()->Mobility != EComponentMobility::Movable)
		{
			//UE_LOG(LogTemp, Warning, TEXT("We have hit the unmovable!"));
			return;
		}

		if (Hit.GetActor()->ActorHasTag(TEXT("IsDoor")))
		{
			m_PhysicsHandle->GrabComponentAtLocation(Hit.GetComponent(), Hit.BoneName, Hit.Location);
		}
		else
		{
			m_PhysicsHandle->GrabComponentAtLocationWithRotation(Hit.GetComponent(), Hit.BoneName, Hit.Location, Hit.GetActor()->GetActorRotation());
		}

		m_PhysicsHandle->SetInterpolationSpeed(m_BaseInterpolationSpeed / Hit.GetComponent()->GetMass());
		m_CurrentGrabDistance = Hit.Distance;
		m_bIsGrabbing = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing hit."));
	}
}

void APhysicsCharacter::ReleaseObject(const FInputActionValue& Value)
{
	// @TODO: Release grabebd object using UPhysicsHandleComponent
	if (m_bIsGrabbing)
	{
		m_PhysicsHandle->ReleaseComponent();

		m_bIsGrabbing = false;
		UE_LOG(LogTemp, Warning, TEXT("Releasing the child."));
	}
}

void APhysicsCharacter::SetHighlightedMesh(UMeshComponent* StaticMesh)
{
	if (m_HighlightedMesh)
	{
		m_HighlightedMesh->SetOverlayMaterial(nullptr);
	}
	m_HighlightedMesh = StaticMesh;
	if (m_HighlightedMesh)
	{
		m_HighlightedMesh->SetOverlayMaterial(m_HighlightMaterial);
	}
}
