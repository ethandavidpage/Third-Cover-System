// Fill out your copyright notice in the Description page of Project Settings.


#include "CoverObject.h"
#include "Cover.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACoverObject::ACoverObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create skeletal mesh
	CoverMesh = CreateDefaultSubobject<UStaticMeshComponent>("CoverMesh");

	//Set to root component
	SetRootComponent(CoverMesh);
	

}

//Returns the furthest cover from the player (Determines which side of cover is opposite to the player)
AActor * ACoverObject::GetFurthestCoverToPlayer()
{	
	FVector playerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FVector coverLoc = MyCovers[0]->GetActorLocation();

	int currentWinner = 0;
	float currentLargestDistance = (coverLoc - playerLoc).SizeSquared();
	float tmpDistance;

	for (int i = 1; i < MyCovers.Num(); i++)
	{
		coverLoc = MyCovers[i]->GetActorLocation();
		tmpDistance = (coverLoc - playerLoc).SizeSquared();
		if (tmpDistance > currentLargestDistance)
		{
			currentLargestDistance = tmpDistance;
			currentWinner = i;
		}
	}
	return MyCovers[currentWinner];

}

// Called when the game starts or when spawned
void ACoverObject::BeginPlay()
{
	Super::BeginPlay();

	//If there is a valid cover template
	if (CoverBP)
	{
		//Rotation of cover mesh initially
		FRotator startRot = GetActorRotation();
		//The cover object is set to zero and the covers are then attached and the whole object rotated back to original rotation. This is to avoid more complex spawning logic
		SetActorRotation(FRotator::ZeroRotator);
		//Covers that are spawned are stored in this tmp variable so they can be edited
		ACover* tmpCover;
		//Parameters for spawn
		FActorSpawnParameters SpawnParam;

		//Stores the size of the mesh to be used for spawning location
		FVector meshSize = CoverMesh->Bounds.GetBox().GetSize();
		//Stores extent of the mesh to be used for changing cover collision box extent
		FVector meshExtent = CoverMesh->Bounds.GetBox().GetExtent();
		//Gets location of the cover mesh
		FVector actorLoc = GetActorLocation();
		//The transform for each cover's spawn
		FTransform SpawnTransform = GetActorTransform();
		//The attachment rules for each cover which are to keep world location, scale, and rotation
		FAttachmentTransformRules rules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
		//Set spawn transform scale to one because the location and bounds of each cover will be determined by mesh size and bounds
		SpawnTransform.SetScale3D(FVector::OneVector);

		//Two for loops for each set of opposite covers (Two on top and bottom, two on left and right of mesh)
		for (int i = -1; i <= 1; i = i + 2)
		{
			//Set location to same Z and Y, but for the X set the  X position + half the mesh size + cover range in either direction; which is what i determines (-1, and 1 )
			SpawnTransform.SetLocation(FVector(actorLoc.X + (((meshSize.X * i) / 2) + (CoverRange * i))  , actorLoc.Y, actorLoc.Z));
			//Spawn the coverBP and store it in tmpcover
			tmpCover = GetWorld()->SpawnActor<ACover>(CoverBP, SpawnTransform, SpawnParam);
			//Set rotation to always be facing towards center of cover mesh (So the forward can be used to see if actors are moving away or towards the cover; if needed)
			tmpCover->SetActorRotation((actorLoc - tmpCover->GetActorLocation()).Rotation());
			//Set cover collision box to match the mesh and stick out by the amount in the coverrange variable
			tmpCover->CollisionBox->SetBoxExtent(FVector(CoverRange, meshExtent.Y, meshExtent.Z));
			//Attach to the root of this object
			tmpCover->AttachToComponent(RootComponent, rules);
			//Add the cover to the local array of covers
			MyCovers.Add(tmpCover);
		}
		//Same as last for loop but for the Y
		for (int i = -1; i <= 1; i = i + 2)
		{
			SpawnTransform.SetLocation(FVector(actorLoc.X, actorLoc.Y + (((meshSize.Y * i) / 2) + (CoverRange * i)), actorLoc.Z));

			tmpCover = GetWorld()->SpawnActor<ACover>(CoverBP, SpawnTransform, SpawnParam);
			tmpCover->SetActorRotation((actorLoc - tmpCover->GetActorLocation()).Rotation());
			tmpCover->CollisionBox->SetBoxExtent(FVector(CoverRange, meshExtent.X, meshExtent.Z));
			tmpCover->AttachToComponent(RootComponent, rules);
			MyCovers.Add(tmpCover);
		}

		//Now all objects are attached and are the right size, rotate the whole object back to original rotation
		SetActorRotation(startRot);
	}

	
}

// Called every frame
void ACoverObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

