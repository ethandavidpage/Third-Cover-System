// Fill out your copyright notice in the Description page of Project Settings.


#include "AIDirector.h"
#include "Kismet/GameplayStatics.h"
#include "CoverObject.h"
#include "Engine/World.h"

// Sets default values
AAIDirector::AAIDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FindAllCovers();
}

float AAIDirector::GetDistanceFromAIToPlayer(FVector pos)
{
	FVector playerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	float dist = (playerLoc - pos).Size();
	return dist;
}

void AAIDirector::FindAllCovers()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACoverObject::StaticClass(), AllCovers);
}

AActor * AAIDirector::GetClosestCover(FVector pos)
{
	FVector coverLoc = AllCovers[0]->GetActorLocation();

	int currentWinner = 0;
	float currentClosestDistance = (coverLoc - pos).Size();
	float tmpDistance;

	for (int i = 1; i < AllCovers.Num(); i++)
	{
		coverLoc = AllCovers[i]->GetActorLocation();
		tmpDistance = (coverLoc - pos).Size();
		if (tmpDistance < currentClosestDistance)
		{
			currentClosestDistance = tmpDistance;
			currentWinner = i;
		}
	}
	return AllCovers[currentWinner];

}

//FIND ALL
TArray<AActor*> AAIDirector::FindAllFlankingCovers()
{
	TArray<AActor*> flankingCovers;
	FVector playerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FVector playerForward = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorForwardVector();
	float distanceBetweenCoverAndPlayer = 0.f;
	float dotProduct = 0.f;
	FVector directionalVectorBetweenPlayerAndCover;
	AActor* tmpCover;

	for (int i = 0; i < AllCovers.Num(); i++)
	{
		tmpCover = AllCovers[i];
		distanceBetweenCoverAndPlayer = (playerLoc - tmpCover->GetActorLocation()).Size();

		//If the distance of the cover to the player is either too far that it would cause the AI to have to advance, or too close that they would have to retreat then it is not valid cover
		if (distanceBetweenCoverAndPlayer < MaxDistanceAwayFromPlayer && distanceBetweenCoverAndPlayer > MinDistanceAwayFromPlayer)
		{
			//Otherwise it is in range and the directional vector needs to be calculated
			directionalVectorBetweenPlayerAndCover = tmpCover->GetActorLocation() - playerLoc;
			directionalVectorBetweenPlayerAndCover.Normalize();
			//Calculate dot product between directional vector and player forward. essentially we are looking for cover objects that are behind the player
			dotProduct = FVector::DotProduct(playerForward, directionalVectorBetweenPlayerAndCover);
			//If dot product is less than zero it means the cover is behind the player
			if (dotProduct < 0)
			{
				flankingCovers.Add(tmpCover);
			}
		}
	}

	return flankingCovers;
}

TArray<AActor*> AAIDirector::FindAllNormalCovers(FVector pos)
{
	TArray<AActor*> normalCovers;
	FVector playerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	float distanceBetweenCoverAndPlayer = 0.f;
	float distanceBetweenCoverAndAI = 0.f;
	AActor* tmpCover;

	for (int i = 0; i < AllCovers.Num(); i++)
	{
		tmpCover = AllCovers[i];
		distanceBetweenCoverAndPlayer = (playerLoc - tmpCover->GetActorLocation()).Size();
		distanceBetweenCoverAndAI = (pos - tmpCover->GetActorLocation()).Size();

		//If the distance of the cover to the player is either too far that it would cause the AI to have to advance, or too close that they would have to retreat then it is not valid cover
		if (distanceBetweenCoverAndPlayer < MaxDistanceAwayFromPlayer && distanceBetweenCoverAndPlayer > MinDistanceAwayFromPlayer)
		{
			if (distanceBetweenCoverAndAI <= 800)
			{
				normalCovers.Add(tmpCover);
			}
		}
	}

	return normalCovers;
}

TArray<AActor*> AAIDirector::FindAllRetreatingCovers(FVector pos, FVector forward)
{
	TArray<AActor*> retreatingCovers;
	FVector playerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	float distanceBetweenCoverAndPlayer = 0.f;
	float dotProduct = 0.f;
	FVector directionalVectorBetweenCoverAndAI;
	AActor* tmpCover;

	for (int i = 0; i < AllCovers.Num(); i++)
	{
		tmpCover = AllCovers[i];
		distanceBetweenCoverAndPlayer = (playerLoc - tmpCover->GetActorLocation()).Size();

		//If the distance of the cover to the player is either too far that it would cause the AI to have to advance, or too close that they would have to retreat then it is not valid cover
		if (distanceBetweenCoverAndPlayer < MaxDistanceAwayFromPlayer && distanceBetweenCoverAndPlayer > MinDistanceAwayFromPlayer)
		{
			//Otherwise it is in range and the directional vector needs to be calculated
			directionalVectorBetweenCoverAndAI = tmpCover->GetActorLocation() - pos;
			directionalVectorBetweenCoverAndAI.Normalize();
			//Calculate dot product between directional vector and AI forward. essentially we are looking for cover objects that are behind the enemy, thus retreating covers
			dotProduct = FVector::DotProduct(forward, directionalVectorBetweenCoverAndAI);
			//If dot product is less than zero it means the cover is behind the AI
			if (dotProduct < 0)
			{
				retreatingCovers.Add(tmpCover);
			}
		}
	}

	return retreatingCovers;


}

TArray<AActor*> AAIDirector::FindAllAdvancingCovers(FVector pos, FVector forward)
{
	TArray<AActor*> advancingCovers;
	FVector playerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	float distanceBetweenCoverAndPlayer = 0.f;
	float dotProduct = 0.f;
	FVector directionalVectorBetweenAIAndPlayer;
	FVector directionalVectorBetweenCoverAndPlayer;
	FVector directionalVectorBetweenCoverAndAI;
	AActor* tmpCover;

	for (int i = 0; i < AllCovers.Num(); i++)
	{
		tmpCover = AllCovers[i];
		distanceBetweenCoverAndPlayer = (playerLoc - tmpCover->GetActorLocation()).Size();

		//If the distance of the cover to the player is either too far that it would cause the AI to have to advance, or too close that they would have to retreat then it is not valid cover
		if (distanceBetweenCoverAndPlayer < MaxDistanceAwayFromPlayer && distanceBetweenCoverAndPlayer > MinDistanceAwayFromPlayer)
		{
			//Otherwise it is in range and the directional vector needs to be calculated
			directionalVectorBetweenCoverAndAI = tmpCover->GetActorLocation() - pos;
			directionalVectorBetweenCoverAndAI.Normalize();
			//Calculate dot product between directional vector and AI forward. to start advancing covers have to not be behind enemy, opposite logic of flanking
			dotProduct = FVector::DotProduct(forward, directionalVectorBetweenCoverAndAI);
			//If dot product is greater than it means the cover is not behind the AI
			if (dotProduct > 0)
			{
				//If the cover is not behind enemy than the next check is if it is inbetween player and AI (Advancing is cover not behind the AI, and cover that is looking at the player in the same direction of the AI thus on the same side of the player)
				
				//Direction enemy is looking to see player
				directionalVectorBetweenAIAndPlayer = playerLoc - pos;
				directionalVectorBetweenAIAndPlayer.Normalize();

				//Direction cover object is looking to see player
				directionalVectorBetweenCoverAndPlayer = playerLoc - tmpCover->GetActorLocation();
				directionalVectorBetweenCoverAndPlayer.Normalize();

				//Calculate dot product between the way cover has to look to see player, and way the AI does, if they are the same it means the cover is not on the other side of the player
				//to the AI (they player is not between them, that mixes with not behind player to ensure advancing cover)
				dotProduct = FVector::DotProduct(directionalVectorBetweenCoverAndPlayer, directionalVectorBetweenAIAndPlayer);

				if (dotProduct > 0)
				{
					advancingCovers.Add(tmpCover);
				}
			}
		}
	}

	return advancingCovers;
}


//GET COVERS
AActor * AAIDirector::GetFlankingCover(FVector pos, AActor * coverAIIsIn)
{
	TArray<AActor*> allFlankingCovers = FindAllFlankingCovers();

	//If there are no valid covers return cover AI is already in so they stay put
	if (allFlankingCovers.Num() == 0)
	{
		return coverAIIsIn;
	}
	else
	{
		//If there is only one cover return that = no distance competition
		if (allFlankingCovers.Num() == 1)
		{
			return allFlankingCovers[0];
		}
		//Calculate each covers distance to the AI and pick the furthest away (as that makes better flanking cover)
		else
		{
			FVector coverLoc = allFlankingCovers[0]->GetActorLocation();
			int currentWinner = 0;
			float currentFurthestDistance = (coverLoc- pos).Size();
			float tmpDistance;

			//For all flanking covers, calculate distance, make it the winner if it is further
			for (int i = 1; i < allFlankingCovers.Num(); i++)
			{
				coverLoc = allFlankingCovers[i]->GetActorLocation();
				tmpDistance = (coverLoc - pos).Size();
				if (tmpDistance > currentFurthestDistance)
				{
					currentFurthestDistance = tmpDistance;
					currentWinner = i;
				}
			}
			return allFlankingCovers[currentWinner];
		}
	}
}

AActor * AAIDirector::GetNormalCover(FVector pos, AActor * coverAIIsIn)
{
	TArray<AActor*> allNormalCovers = FindAllNormalCovers(pos);

	//If there are no valid covers return cover AI is already in so they stay put
	if (allNormalCovers.Num() == 0)
	{
		return coverAIIsIn;
	}
	else
	{
		//If there is only one cover return that = no distance competition
		if (allNormalCovers.Num() == 1)
		{
			return allNormalCovers[0];
		}
		//Calculate each covers distance to the AI and pick the closest one
		else
		{
			FVector coverLoc = allNormalCovers[0]->GetActorLocation();
			int currentWinner = 0;
			float currentClosestDistance = (coverLoc - pos).Size();
			float tmpDistance;

			//For all normal covers, calculate distance, make it the winner if it is further
			for (int i = 1; i < allNormalCovers.Num(); i++)
			{
				coverLoc = allNormalCovers[i]->GetActorLocation();
				tmpDistance = (coverLoc - pos).Size();
				if (tmpDistance < currentClosestDistance)
				{
					currentClosestDistance = tmpDistance;
					currentWinner = i;
				}
			}
			return allNormalCovers[currentWinner];
		}
	}
}

AActor * AAIDirector::GetRetreatingCover(FVector pos, FVector forward, AActor * coverAIIsIn)
{
	TArray<AActor*> allRetreatingCovers = FindAllRetreatingCovers(pos, forward);

	//If there are no valid covers return cover AI is already in so they stay put
	if (allRetreatingCovers.Num() == 0)
	{
		return coverAIIsIn;
	}
	//Calculate each covers distance to the AI and pick the closest one
	else
	{
		//If there is only one cover return that = no distance competition
		if (allRetreatingCovers.Num() == 1)
		{
			return allRetreatingCovers[0];
		}
		else
		{
			FVector coverLoc = allRetreatingCovers[0]->GetActorLocation();
			int currentWinner = 0;
			float currentClosestDistance = (coverLoc - pos).Size();
			float tmpDistance;

			//For all normal covers, calculate distance, make it the winner if it is further
			for (int i = 1; i < allRetreatingCovers.Num(); i++)
			{
				coverLoc = allRetreatingCovers[i]->GetActorLocation();
				tmpDistance = (coverLoc - pos).Size();
				if (tmpDistance < currentClosestDistance)
				{
					currentClosestDistance = tmpDistance;
					currentWinner = i;
				}
			}
			return allRetreatingCovers[currentWinner];
		}
	}
}

AActor * AAIDirector::GetAdvancingCover(FVector pos, FVector forward, AActor * coverAIIsIn)
{
	TArray<AActor*> allAdvancingCovers = FindAllAdvancingCovers(pos, forward);

	//If there are no valid covers return cover AI is already in so they stay put
	if (allAdvancingCovers.Num() == 0)
	{
		return coverAIIsIn;
	}
	//Calculate each covers distance to the AI and pick the closest one
	else
	{
		//If there is only one cover return that = no distance competition
		if (allAdvancingCovers.Num() == 1)
		{
			return allAdvancingCovers[0];
		}
		else
		{
			FVector coverLoc = allAdvancingCovers[0]->GetActorLocation();
			int currentWinner = 0;
			float currentClosestDistance = (coverLoc - pos).Size();
			float tmpDistance;

			//For all normal covers, calculate distance, make it the winner if it is further
			for (int i = 1; i < allAdvancingCovers.Num(); i++)
			{
				coverLoc = allAdvancingCovers[i]->GetActorLocation();
				tmpDistance = (coverLoc - pos).Size();
				if (tmpDistance < currentClosestDistance)
				{
					currentClosestDistance = tmpDistance;
					currentWinner = i;
				}
			}
			return allAdvancingCovers[currentWinner];
		}
	}
}


AActor * AAIDirector::GetCover(FVector pos, FVector forward, AActor * coverAIIsIn, MovementTypes movementType)
{
	AActor* tmpCover;
	if (coverAIIsIn)
	{
		//Only add back cover if it is valid
		AllCovers.Add(coverAIIsIn);
	}
	else
	{
		//If there is no valid cover, as in AI's first choice, then get closest cover
		coverAIIsIn = GetClosestCover(pos);
	}
	//Switch on all four types, for each get and then return most appropriate cover of wanted type, then remove that cover from array of all covers
	switch (movementType)
	{
	case MovementTypes::Normal:
		tmpCover = GetNormalCover(pos, coverAIIsIn);
		AllCovers.Remove(tmpCover);
		return tmpCover;
	case MovementTypes::Advancing:
		tmpCover = GetAdvancingCover(pos, forward, coverAIIsIn);
		AllCovers.Remove(tmpCover);
		return tmpCover;
	case MovementTypes::Flanking:
		tmpCover = GetFlankingCover(pos, coverAIIsIn);
		AllCovers.Remove(tmpCover);
		return tmpCover;
	case MovementTypes::Retreating:
		tmpCover = GetRetreatingCover(pos, forward, coverAIIsIn);
		AllCovers.Remove(tmpCover);
		return tmpCover;
	default:
		return coverAIIsIn;
	}
}

// Called when the game starts or when spawned
void AAIDirector::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAIDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

