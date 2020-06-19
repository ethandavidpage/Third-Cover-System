// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GunslingersCharacter.generated.h"

UCLASS(config=Game)
class AGunslingersCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AGunslingersCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;










protected:


	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
		


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	

	void SetInCover();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Cover)
	class UBoxComponent* CollisionProbe;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Cover)
	class UBoxComponent* OutOfCoverCollisionProbe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cover)
	AActor* CurrentCover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class AWeapon* EquipedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cover)
	bool IsInCover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	bool IsAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	bool IsCrouching;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	bool IsSprinting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
	bool IsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Control)
	bool IsPaused = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SlowMo)
	bool IsSlowMo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SlowMo)
	float SlowMoAmount = 3.f;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Cover();	

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Aim();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopAim();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void CrouchButton();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ShootWeaponButton();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ReloadWeaponButton();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EndSprint();

	UFUNCTION(BlueprintCallable, Category = "SlowMotion")
	void SlowMotion();

	UFUNCTION(BlueprintCallable, Category = "Control")
	void Menu();

	int CalculateClosestCover(TArray<AActor*> covers);
	


		


public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual FVector GetPawnViewLocation() const override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;



};

