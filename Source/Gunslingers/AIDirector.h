// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIDirector.generated.h"

UENUM(BlueprintType)
enum MovementTypes
{
	Normal UMETA(DisplayName = "Normal"),
	Flanking UMETA(DisplayName = "Flanking"),
	Advancing UMETA(DisplayName = "Advancing"),
	Retreating UMETA(DisplayName = "Retreating")
};

UCLASS()
class GUNSLINGERS_API AAIDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAIDirector();	


	//Stores a reference to each of the cover objects around the cover mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TArray<AActor*> AllCovers;

	//Stores a reference to each of the cover objects around the cover mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float MinDistanceAwayFromPlayer = 400.f;

	//Stores a reference to each of the cover objects around the cover mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float MaxDistanceAwayFromPlayer = 1500.f;

	UFUNCTION(BlueprintCallable)
	float GetDistanceFromAIToPlayer(FVector pos);



	UFUNCTION()
	void FindAllCovers();

	UFUNCTION(BlueprintCallable)
	AActor* GetClosestCover(FVector pos);

	//FIND ALLS

	UFUNCTION(BlueprintCallable)
	TArray<AActor*> FindAllFlankingCovers();

	UFUNCTION(BlueprintCallable)
	TArray<AActor*> FindAllNormalCovers(FVector pos);

	UFUNCTION(BlueprintCallable)
	TArray<AActor*> FindAllRetreatingCovers(FVector pos, FVector forward);

	UFUNCTION(BlueprintCallable)
	TArray<AActor*> FindAllAdvancingCovers(FVector pos, FVector forward);

	//GET COVERS

	UFUNCTION(BlueprintCallable)
	AActor* GetFlankingCover(FVector pos, AActor* coverAIIsIn );

	UFUNCTION(BlueprintCallable)
	AActor* GetNormalCover(FVector pos, AActor* coverAIIsIn);

	UFUNCTION(BlueprintCallable)
	AActor* GetRetreatingCover(FVector pos, FVector forward, AActor* coverAIIsIn);

	UFUNCTION(BlueprintCallable)
	AActor* GetAdvancingCover(FVector pos, FVector forward, AActor* coverAIIsIn);


	//This last function will be the only outside called function and will take an enum type {retreating, advancing, flanking, normal}

	UFUNCTION(BlueprintCallable)
	AActor* GetCover(FVector pos, FVector forward, AActor* coverAIIsIn, MovementTypes movementType);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

