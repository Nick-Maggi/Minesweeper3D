// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Minesweeper3DBlock.generated.h"

/** A block that can be clicked */
UCLASS(minimalapi)
class AMinesweeper3DBlock : public AActor
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

	/** StaticMesh component for the clickable block */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BlockMesh;

public:
	AMinesweeper3DBlock();

	/** Are we currently active? */
	enum State{hidden, revealed, flagged};

	State BlockState = hidden;

	bool bIsMine;

	bool bIsRevealed;

	UPROPERTY()
	int Xpos;

	UPROPERTY()
	int Ypos;

	UPROPERTY()
	int Zpos;

	//Only mines should have a -1 after all the blocks are updated
	int NumSurroundingMines = -1;

	/** Pointer to white material used on the focused block */
	UPROPERTY()
	class UMaterial* BaseMaterial;

	/** Pointer to blue material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* BlueMaterial;

	/** Pointer to orange material used on active blocks */
	UPROPERTY()
	class UMaterialInstance* OrangeMaterial;

	class UStaticMesh* FlagMesh;

	class UStaticMesh* DefaultMesh;

	/** Grid that owns us */
	UPROPERTY()
	class AMinesweeper3DBlockGrid* OwningGrid;

	/** Handle the block being clicked */
	UFUNCTION()
	void BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked);

	/** Handle the block being touched  */
	UFUNCTION()
	void OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);

	void Reveal();

	void Flag();

	void Highlight(bool bOn);

	void RevealAdjacentBlocks();

public:
	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
	/** Returns BlockMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBlockMesh() const { return BlockMesh; }
};



