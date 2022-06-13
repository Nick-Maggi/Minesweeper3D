// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "Minesweeper3DGameMode.generated.h"

/** GameMode class to specify pawn and playercontroller */
UCLASS(minimalapi)
class AMinesweeper3DGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMinesweeper3DGameMode();

protected:
	virtual void BeginPlay() override;
};



