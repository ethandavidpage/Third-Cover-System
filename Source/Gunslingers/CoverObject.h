// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoverObject.generated.h"

UCLASS()
class GUNSLINGERS_API ACoverObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACoverObject();

	//Stores a reference to the coverBP that should be spawned around the mesh
	UPROPERTY(EditAnywhere, Category = "Components")
	TSubclassOf<class ACover> CoverBP;

	//Stores a reference to each of the cover objects around the cover mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<AActor*> MyCovers;

	//Mesh declaration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* CoverMesh;

	//This is how far each cover object will stick out from the mesh, and therefore the range the player can enter cover/walk away from cover before exiting it
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	float CoverRange = 10.f;

	//Can be called by ai to recieve the furthest cover object to player (This will be the one opposite the player, so that they are covered from player)
	UFUNCTION(BlueprintCallable, Category = "Utility")
	AActor* GetFurthestCoverToPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
