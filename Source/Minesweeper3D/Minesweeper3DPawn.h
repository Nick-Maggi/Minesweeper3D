// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Minesweeper3DPawn.generated.h"

UCLASS(config=Game)
class AMinesweeper3DPawn : public APawn
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	void RotateLeftRight(float AxisVal);
	void RotateUpDown(float AxisVal);

protected:
	void OnResetVR();
	void TriggerClick();
	void TraceForBlock(const FVector& Start, const FVector& End, bool bDrawDebugHelpers);


	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	class AMinesweeper3DBlock* CurrentBlockFocus;
};
