// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GunslingersCharacter.h"


// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create skeletal mesh
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	
	//Set to root
	SetRootComponent(MeshComponent);
}

void AWeapon::FireWeapon()
{
	//If not reloading
	if (!IsReloading)
	{
		//Check if current ammo is zero and there is some ammo before attempting a reload, else the shot takes one round. Don't want to needlessly call the reloadWeapon function if there is no ammo
		if (CurrentAmmo <= 0)
		{
			//If there is total ammo
			if (TotalAmmo != 0)
			{
				ReloadWeapon();
			}
		}
		//Shooting can only occur with ammo
		else
		{
			CurrentAmmo -= 1;

			MeshComponent->PlayAnimation(FireAnim,false);

			//Ray trace from player to crosshair location in space
			AActor* weaponOwner = GetOwner();
			if (weaponOwner)
			{
				//Two types of shooting, from the player or the enemy
				if (!IsEnemies)
				{
					//Direction and points of trace
					FVector startPoint;
					FRotator rotation;
					weaponOwner->GetActorEyesViewPoint(startPoint, rotation);

					FVector shotDirection = rotation.Vector();

					FVector endPoint = startPoint + (shotDirection * 100000);

					//Trace parameters
					FCollisionQueryParams collisionParam;
					collisionParam.AddIgnoredActor(weaponOwner);
					collisionParam.AddIgnoredActor(this);
					collisionParam.bTraceComplex = true;

					//Perform trace and store result in hit
					FHitResult hit;
					if (GetWorld()->LineTraceSingleByChannel(hit, startPoint, endPoint, ECC_Visibility, collisionParam))
					{
						AActor* hitActor = hit.GetActor();
						UGameplayStatics::ApplyPointDamage(hitActor, WeaponDamage, shotDirection, hit, weaponOwner->GetInstigatorController(), this, DamageType);

						//Any visual feedback or bullet line will go from the barrel to the final hit location but the actual line trace is from the camera; for now commented out

						/*startPoint = MeshComponent->GetSocketLocation("MuzzleFlash");
						endPoint = hit.Location;
						DrawDebugLine(GetWorld(), startPoint, endPoint, FColor::Green, false, 1.0f, 0, 1.0f);*/

					}					
				}
				else
				{
					//Direction and points of trace
					FVector startPoint = weaponOwner->GetActorLocation() + (FVector::UpVector * 50);
					AGunslingersCharacter* player = Cast<AGunslingersCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
					float playerVelocity = player->GetVelocity().X;
					FVector endPoint = player->GetMesh()->GetBoneLocation("spine_03");
					FVector randomOffset = FVector(FMath::FRandRange(-Inaccuracy - playerVelocity, Inaccuracy + playerVelocity), FMath::FRandRange(-Inaccuracy - playerVelocity, Inaccuracy + playerVelocity), FMath::FRandRange(-Inaccuracy - playerVelocity, Inaccuracy + playerVelocity));
					endPoint += randomOffset;
					FVector direction = endPoint - startPoint;
					endPoint = startPoint + (direction * 100000);

					//Trace parameters
					FCollisionQueryParams collisionParam;
					collisionParam.AddIgnoredActor(weaponOwner);
					collisionParam.AddIgnoredActor(this);
					collisionParam.bTraceComplex = true;

					//Perform trace and store result in hit
					FHitResult hit;
					if (GetWorld()->LineTraceSingleByChannel(hit, startPoint, endPoint, ECC_Visibility, collisionParam))
					{
						AActor* hitActor = hit.GetActor();
						UGameplayStatics::ApplyPointDamage(hitActor, WeaponDamage, GetActorForwardVector(), hit, weaponOwner->GetInstigatorController(), this, DamageType);

						//Any visual feedback or bullet line will go from the barrel to the final hit location but the actual line trace is from the camera; for now commented out


						//startPoint = MeshComponent->GetSocketLocation("MuzzleFlash");
						//endPoint = hit.Location;
						//DrawDebugLine(GetWorld(), startPoint, endPoint, FColor::Green, false, 1.0f, 0, 1.0f);

					}
				}
			}
		}
	}
}

//Reload logic
void AWeapon::ReloadWeapon()
{
	//If current ammo is not zero than a reload can occur, the check may seem redundant as in Fire this check is done, but ReloadWeapon can be called by a reload input event as well.
	//Check is also redundant if no rounds have been fired
	if (TotalAmmo != 0 && CurrentAmmo != MagazineSize && !IsReloading)
	{
		IsReloading = true;
		GetWorldTimerManager().SetTimer(reloadTimerHandle, this, &AWeapon::OnTimerEnd, 1.6f, false );
	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

//In reload you start the timer, once it ends you do the actual reload
void AWeapon::OnTimerEnd()
{
	//Give enemy infinite ammo
	if (IsEnemies)
	{
		CurrentAmmo = MagazineSize;
	}
	else
	{
		//Calculate amount of rounds spent
		int AmmoSpent = MagazineSize - CurrentAmmo;
		//If the total ammo can cover the amount of rounds spent
		if (TotalAmmo >= AmmoSpent)
		{
			TotalAmmo = TotalAmmo - (MagazineSize - CurrentAmmo);
			CurrentAmmo = CurrentAmmo + (MagazineSize - CurrentAmmo);
		}
		else
		{
			CurrentAmmo += TotalAmmo;
			TotalAmmo = 0;
		}
	}
	GetWorldTimerManager().ClearTimer(reloadTimerHandle);
	IsReloading = false;
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


