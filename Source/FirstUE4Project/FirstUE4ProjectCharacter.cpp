// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirstUE4ProjectCharacter.h"
#include "FirstUE4ProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AFirstUE4ProjectCharacter

AFirstUE4ProjectCharacter::AFirstUE4ProjectCharacter(const FObjectInitializer& ObjectInitializer) : ANinjaCharacter(ObjectInitializer)
{
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	//FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

}

void AFirstUE4ProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		// VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		// VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFirstUE4ProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFirstUE4ProjectCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFirstUE4ProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFirstUE4ProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AFirstUE4ProjectCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFirstUE4ProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AFirstUE4ProjectCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFirstUE4ProjectCharacter::LookUpAtRate);
}

void AFirstUE4ProjectCharacter::OnFire() {
	// try and fire a projectile
	if (ProjectileClass != nullptr)	{
		UWorld* const World = GetWorld();
		if (World != nullptr) {
			const FRotator SpawnRotation = FP_MuzzleLocation->GetComponentRotation();
			const FVector SpawnLocation = FP_MuzzleLocation->GetComponentLocation();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// spawn the projectile at the muzzle
			World->SpawnActor<AFirstUE4ProjectProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// try and play the sound if specified
	if (FireSound != nullptr) {
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr) {
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr) {
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}


void AFirstUE4ProjectCharacter::MoveForward(float Value) {
	if (Value != 0.0f) {
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFirstUE4ProjectCharacter::MoveRight(float Value){
	if (Value != 0.0f) {
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFirstUE4ProjectCharacter::Turn(float Rate) {
	// calculate delta for this frame from the rate information
	// AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	const auto rot = FRotator(0, Rate * 100 * GetWorld()->GetDeltaSeconds(), 0);
	AddActorLocalRotation(rot);
}

void AFirstUE4ProjectCharacter::TurnAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	// AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	const auto rot = FRotator(0, Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds(), 0);
	AddActorLocalRotation(rot);
}

void AFirstUE4ProjectCharacter::LookUp(float Rate) {
	// calculate delta for this frame from the rate information
	//AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	const auto rot = FRotator(-Rate * 100 * GetWorld()->GetDeltaSeconds(), 0, 0);
	FirstPersonCameraComponent->AddLocalRotation(rot);
}

void AFirstUE4ProjectCharacter::LookUpAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	//AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	const auto rot = FRotator(-Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds(), 0, 0);
	FirstPersonCameraComponent->AddLocalRotation(rot);
}
