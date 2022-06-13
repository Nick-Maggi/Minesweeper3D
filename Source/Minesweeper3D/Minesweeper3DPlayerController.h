// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Minesweeper3DPlayerController.generated.h"

/** PlayerController class used to enable cursor */
UCLASS()
class AMinesweeper3DPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMinesweeper3DPlayerController();

	virtual void BeginPlay() override;
};


