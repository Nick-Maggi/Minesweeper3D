// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Containers/Array.h"
#include "Minesweeper3DBlock.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/CheckBox.h"
#include "Minesweeper3DBlockGrid.generated.h"

/** Class used to spawn blocks and manage score */
UCLASS(minimalapi)
class AMinesweeper3DBlockGrid : public APawn
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

public:
	AMinesweeper3DBlockGrid();

	/** Number of blocks along each side of grid */
	UPROPERTY(Category = Grid, EditAnywhere, BlueprintReadOnly)
	int32 Size = 0;

	float MinesPercentage = 0.068;
	int NumMines = 0;

	//The number displaying how many mines the player has yet to find
	UPROPERTY(BlueprintReadOnly)
	int MinesRemaining = 0;

	//Used to check win condition (blocks remaining = num mines)
	int BlocksRemaining = 0;

	UPROPERTY(BlueprintReadOnly)
	int ElapsedTime = 0;

	/** Spacing of blocks */
	UPROPERTY(Category=Grid, EditAnywhere, BlueprintReadOnly)
	float BlockSpacing;

	//3D array of all the blocks this grid manages
	TArray< TArray< TArray< AMinesweeper3DBlock*> > > Blocks;

	//Used to randomize all the blocks and choose some number to be mines
	TArray<AMinesweeper3DBlock*> BlockList;

	//List of meshes for each case of 1-26 mines surrounding a block. Index 0 is the mesh for a mine, all the others are assigned numerically
	TArray<UStaticMesh*> NumberFaces;

	//Reference to the camera we grab from the scene in BeginPlay
	AActor* Camera;

	//Camera location and rotation variables
	float theta;
	float phi;
	float radius;
	FVector CubeCenter;

	FVector TranslationInputDirection;

	//Overlay widgets
	TSubclassOf<UUserWidget> SettingsWidget;
	TSubclassOf<UUserWidget> HUDWidget;
	TSubclassOf<UUserWidget> CustomSettingsWidget;
	UUserWidget* ActiveWidget;
	UUserWidget* SecondaryWidget;

	FTimerHandle GameClockTimer;

	//The rate at which the camera zooms, used in ZoomCamera()
	float ZoomSpeed = 20.0f;
	//Whether this is the first block clicked or not. If so, generate the mines so as to avoid the first block being a mine.
	bool bFirstClick = true;
	//Whether this is the first game or not. If so, generate blocks when we close the menu for the first time.
	bool bFirstGame = true;
	//Whether we're in the settings menu or using the HUD
	bool bIsSettings;
	//Whether the game has been lost yet or not
	UPROPERTY(BlueprintReadOnly)
	bool bGameLost = false;
	//Whether the game has been won or not
	UPROPERTY(BlueprintReadOnly)
	bool bGameWon = false;
	
	bool bIsFreeCam = false;

protected:
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UMG Game")
	TSubclassOf<UUserWidget> StartingWidgetClass;

	UPROPERTY()
	UUserWidget* CurrentWidget;

private:
	void GenerateBlocks();
	void GenerateMines();
	void AssignSurroundingMineTotals();
	void DestroyBlocks();
	void SafelockBlocks(AMinesweeper3DBlock* block);

public:

	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void ZoomIn();
	void ZoomOut();
	void ZoomStop();
	void ZoomCamera(bool bZoomOut);
	void ResetCameraPosition();
	void UpdateCameraPosition();
	float DistanceFromCenter();
	void ChangeTheta(float AxisValue);
	void ChangePhi(float AxisValue);
	void MoveUpDown(float AxisValue);
	void MoveLeftRight(float AxisValue);
	void MoveInOut(float AxisValue);

	void AdvanceTimer();

	UFUNCTION(BlueprintCallable, Category = "UMG Game")
	void ToggleSettings();

	UFUNCTION(BlueprintCallable, Category = "UMG Game")
	void ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass);

	UFUNCTION(BlueprintCallable, Category = "UMG Game")
	void CloseMenu();

	UFUNCTION(BlueprintCallable, Category = "UMG Game")
	void GetActiveCheckbox(UCheckBox* box);

	UFUNCTION(BlueprintCallable, Category = "UMG Game")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "UMG Game")
	void ChangeSize(FString NewSize);

	UFUNCTION(BlueprintCallable, Category = "UMG Game")
	void ChangeMines(FString NewMines);
	

	bool CheckBlockBounds(int Xpos, int Ypos, int Zpos);
	int CalcSurroundingMines(int Xpos, int Ypos, int Zpos);
	void FinishSetup(AMinesweeper3DBlock *block);
	void CheckForWin();
	void RevealMines();

	
};



