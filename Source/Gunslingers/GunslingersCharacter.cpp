// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GunslingersCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h" 
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Weapon.h"
#include "Cover.h"
#include "CoverObject.h"

//////////////////////////////////////////////////////////////////////////
// AGunslingersCharacter

AGunslingersCharacter::AGunslingersCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);	

	// set our turn rates for input
	BaseTurnRate = 50.f;
	BaseLookUpRate = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//Probe for when in cover and for going to cover
	CollisionProbe = CreateDefaultSubobject<UBoxComponent>("CollisionProbe");
	CollisionProbe->SetBoxExtent(FVector(150.f, 2.f, 2.f));

	//Probe for when crouching outside of cover
	OutOfCoverCollisionProbe = CreateDefaultSubobject<UBoxComponent>("OutOfCoverProbe");
	CollisionProbe->SetBoxExtent(FVector(80.f, 2.f, 2.f));



	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGunslingersCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//Custom bindings for cover, aim, sprint, crouch, shoot, reload, slowmotion, and menu
	PlayerInputComponent->BindAction("Cover", IE_Pressed, this, &AGunslingersCharacter::Cover);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AGunslingersCharacter::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AGunslingersCharacter::StopAim);

	PlayerInputComponent->BindAction("SlowMotion", IE_Pressed, this, &AGunslingersCharacter::SlowMotion);

	FInputActionBinding& pause = PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &AGunslingersCharacter::Menu);
	pause.bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AGunslingersCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AGunslingersCharacter::EndSprint);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AGunslingersCharacter::CrouchButton);

	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AGunslingersCharacter::ShootWeaponButton);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AGunslingersCharacter::ReloadWeaponButton);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGunslingersCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGunslingersCharacter::MoveRight);


	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGunslingersCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGunslingersCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AGunslingersCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AGunslingersCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AGunslingersCharacter::OnResetVR);
}

void AGunslingersCharacter::BeginPlay()
{
	Super::BeginPlay();

}

//Sets player to be in cover and moves them to it
void AGunslingersCharacter::SetInCover()
{
	IsInCover = true;
	UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), CurrentCover);	
	Crouch();
	IsCrouching = true;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}


void AGunslingersCharacter::Cover()
{
	//UE_LOG(LogTemp, Warning, TEXT("Cover pressed"));
	//Get all cover actors overlapping the probe
	TArray<AActor*> CoverObjects;
	CollisionProbe->GetOverlappingActors(CoverObjects, ACover::StaticClass());

	//If there are no cover objects it means the player is not looking at their own or a new cover, so must leave cover.
	if (CoverObjects.Num() == 0)
	{
		CurrentCover = nullptr;
		IsInCover = false;
	}
	//If there is a current cover
	else if (IsInCover)
	{
		//If the number of cover objects is 2 or more there is potential for the array to contain current cover by mistake, covers should be then picked on proximity
		if (CoverObjects.Num() >= 2)
		{
			//If current cover is in there remove it
			if (CoverObjects.Contains(CurrentCover))
			{
				CoverObjects.Remove(CurrentCover);
			}
			
			int closestCover = 0;

			//If after the current cover is removed or not removed there is still 2 potential covers there distances need to be checked; otherwise there was only one valid cover and our own in which case there is no competition
			if (CoverObjects.Num() >= 2)
			{
				closestCover = CalculateClosestCover(CoverObjects);
			}

			CurrentCover = CoverObjects[closestCover];
			SetInCover();
		}
		//Else there is only one cover being looked at which is either our own or a new one
		else
		{
			//If the current cover is still equal to the potential cover, it means the player is only looking at their own cover and wants to exit 
			if (CurrentCover == CoverObjects[0])
			{
				CurrentCover = nullptr;
				IsInCover = false;
			}
			//Else the potential cover is new, and the player should move to it
			else
			{
				CurrentCover = CoverObjects[0];						
				SetInCover();
			}
		}
	}
	//Else there is a potential cover but the player is not in cover, just set cover to closest potential
	else
	{
		int closestCover = 0;
		//If more than or equal to 2 a distance check needs to be done
		if (CoverObjects.Num() >= 2)
		{
			closestCover = CalculateClosestCover(CoverObjects);
		}
		CurrentCover = CoverObjects[closestCover];	
		SetInCover();
	}
	

}

void AGunslingersCharacter::Aim()
{
	FollowCamera->SetFieldOfView(70.f);
	IsAiming = true;
	if (IsInCover)
	{
		//If aiming is clicked there is a check to see if the probe is overlapping a cover mesh, if so we want to stand to see over it
		TArray<AActor*> CoverObjects;
		CollisionProbe->GetOverlappingActors(CoverObjects, ACoverObject::StaticClass());
		if (CoverObjects.Num() > 0)
		{
			UnCrouch();
			IsCrouching = false;
			GetCharacterMovement()->MaxWalkSpeed = 600.f;
		}
	}

}

void AGunslingersCharacter::StopAim()
{
	//When aim is let go, set fov back to normal and if in cover set back to crouching
	FollowCamera->SetFieldOfView(90.f);
	IsAiming = false;
	if (IsInCover)
	{
		Crouch();
		IsCrouching = true;
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}
}

void AGunslingersCharacter::CrouchButton()
{
	if (IsCrouching)
	{
		UnCrouch();
		IsCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
	else
	{
		Crouch();
		IsCrouching = true;
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}
}

void AGunslingersCharacter::ShootWeaponButton()
{
	if (EquipedWeapon && !IsSprinting)
	{
		if (IsCrouching)
		{
			if (IsInCover)
			{
				//Check to make sure we are not overlapping any covers, or at least not our own, before shooting while in cover
				TArray<AActor*> CoverObjects;
				AActor* parent = CurrentCover->GetAttachParentActor();
				CollisionProbe->GetOverlappingActors(CoverObjects, ACoverObject::StaticClass());
				if (CoverObjects.Num() < 1)
				{
					EquipedWeapon->FireWeapon();
				}
				//Or does not contain parent
				else if (!CoverObjects.Contains(parent))
				{
					EquipedWeapon->FireWeapon();
				}
			}
			else
			{
				//If not in cover but still crouching use different probe, with overlapping any cover mesh stopping shooting
				TArray<AActor*> CoverObjects;
				OutOfCoverCollisionProbe->GetOverlappingActors(CoverObjects, ACoverObject::StaticClass());
				if (CoverObjects.Num() < 1)
				{
					EquipedWeapon->FireWeapon();
				}
			}
		}
		else
		{
			EquipedWeapon->FireWeapon();
		}
	}
}

void AGunslingersCharacter::ReloadWeaponButton()
{
	if (EquipedWeapon && !IsSprinting)
	{
		EquipedWeapon->ReloadWeapon();
	}
}

void AGunslingersCharacter::StartSprint()
{
	IsSprinting = true;
	if (IsCrouching)
	{
		UnCrouch();
		IsCrouching = false;		
	}
	GetCharacterMovement()->MaxWalkSpeed = 800.f;
}

void AGunslingersCharacter::EndSprint()
{
	IsSprinting = false;
	if (IsInCover)
	{
		Crouch();
		IsCrouching = true;
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
}

void AGunslingersCharacter::SlowMotion()
{
	//If not slow mo, set to slow mo
	if (!IsSlowMo && SlowMoAmount >= 1.5f)
	{
		IsSlowMo = true;
		GetWorldSettings()->SetTimeDilation(0.4f);
	}
	else
	{
		IsSlowMo = false;
		GetWorldSettings()->SetTimeDilation(1.f);
	}
}

void AGunslingersCharacter::Menu()
{
	if (IsPaused)
	{
		IsPaused = false;
		APlayerController* pc = Cast<APlayerController>(GetController());
		pc->bShowMouseCursor = false;
		pc->bEnableClickEvents = false;
		pc->bEnableMouseOverEvents = false;
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	else
	{
		IsPaused = true;
		APlayerController* pc = Cast<APlayerController>(GetController());
		pc->bShowMouseCursor = true;
		pc->bEnableClickEvents = true;
		pc->bEnableMouseOverEvents = true;
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

//Used to determine which side the player ought to move to 
int AGunslingersCharacter::CalculateClosestCover(TArray<AActor*> covers)
{
	//Find smallest distance, by default this will be the first
	float smallestDistance = (GetActorLocation() - covers[0]->GetActorLocation()).SizeSquared();
	int arrayPointer = 0;


	for (int i = 1; i < covers.Num(); i++)
	{
		float tmpDistance = (GetActorLocation() - covers[i]->GetActorLocation()).SizeSquared();
		//If closer than best make the current 'winner'
		if (tmpDistance < smallestDistance)
		{
			smallestDistance = tmpDistance;
			arrayPointer = i;
		}
	}


	return arrayPointer;
}


FVector AGunslingersCharacter::GetPawnViewLocation() const
{
	if (FollowCamera)
	{
		return FollowCamera->GetComponentLocation();
	}	

	return Super::GetPawnViewLocation();
}

void AGunslingersCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Slow mo either drains or regens over time
	if (IsSlowMo)
	{
		if (SlowMoAmount > 0)
		{
			SlowMoAmount -= DeltaTime;
		}
		else
		{
			IsSlowMo = false;
			GetWorldSettings()->SetTimeDilation(1.f);
		}
	}
	else
	{
		if (SlowMoAmount < 3.f)
		{
			SlowMoAmount += DeltaTime;
		}
	}

	//Can regen half health back
	if (Health < 50.f && Health > 0)
	{
		Health += DeltaTime * 5;
	}
}


void AGunslingersCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AGunslingersCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AGunslingersCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}


void AGunslingersCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGunslingersCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AGunslingersCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		//If there is a valid path return
		if (UAIBlueprintHelperLibrary::GetCurrentPath(GetController()))
		{
			return;
		}
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);

	}
}

void AGunslingersCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f))
	{
		//If there is a valid path return
		if (UAIBlueprintHelperLibrary::GetCurrentPath(GetController()))
		{
			return;
		}
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction

		AddMovementInput(Direction, Value);
	}
}



