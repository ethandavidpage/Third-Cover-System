// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class GUNSLINGERS_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* MeshComponent;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	

	//Declaration of damage type
	UPROPERTY(EditAnywhere, Category = "Weapon")
	class TSubclassOf<UDamageType> DamageType;
	
	//Declaration of current ammo variable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float Inaccuracy = 100.f;

	//Declaration of current ammo variable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int CurrentAmmo = 15;

	//Declaration of total ammo variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int TotalAmmo = 60;

	//If the weapon is the enemies the raycasting is different
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool IsEnemies = false;

	//Declaration of magazine size variable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int MagazineSize = 15;

	//Declaration of fire rate for AI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float fireRate = 0.5f;

	//Amount of damage weapon does
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float WeaponDamage = 33.f;

	//Animation that plays when the weapon fires
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UAnimSequence* FireAnim;

	//Is reloading, these two will eventually need to be VisibleAnywhere not edit but for testing leave it
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool IsReloading = false;

	//Timer for reload delay
	FTimerHandle reloadTimerHandle;

	//Logic for when the timer ends (Reload logic)
	UFUNCTION()
	void OnTimerEnd();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Declaration of fire function
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FireWeapon();

	//Declaration of Reload function
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ReloadWeapon();

};
