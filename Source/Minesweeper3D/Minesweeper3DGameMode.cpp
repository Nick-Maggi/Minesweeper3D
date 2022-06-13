// Copyright Epic Games, Inc. All Rights Reserved.

#include "Minesweeper3DGameMode.h"
#include "Minesweeper3DPlayerController.h"
#include "Minesweeper3DPawn.h"
#include "Minesweeper3DBlockGrid.h"

AMinesweeper3DGameMode::AMinesweeper3DGameMode()
{
	// no pawn by default
	//DefaultPawnClass = AMinesweeper3DPawn::StaticClass();
	DefaultPawnClass = AMinesweeper3DBlockGrid::StaticClass();
	// use our own player controller class
	PlayerControllerClass = AMinesweeper3DPlayerController::StaticClass();
}

void AMinesweeper3DGameMode::BeginPlay()
{
	Super::BeginPlay();
}



