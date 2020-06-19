// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GunslingersGameMode.generated.h"

UCLASS(minimalapi)
class AGunslingersGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGunslingersGameMode();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIDirector* AIDirector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	int AliveEnemyCount;
	
};



