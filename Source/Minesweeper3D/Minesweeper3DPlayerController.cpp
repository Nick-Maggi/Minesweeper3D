// Copyright Epic Games, Inc. All Rights Reserved.

#include "Minesweeper3DPlayerController.h"

AMinesweeper3DPlayerController::AMinesweeper3DPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	ClickEventKeys.Add("LeftMouseButton");
	ClickEventKeys.Add("RightMouseButton");
}

void AMinesweeper3DPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetInputMode(FInputModeGameAndUI());
}