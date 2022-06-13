// Copyright Epic Games, Inc. All Rights Reserved.

#include "Minesweeper3DBlock.h"
#include "Minesweeper3DBlockGrid.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstance.h"

AMinesweeper3DBlock::AMinesweeper3DBlock()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		ConstructorHelpers::FObjectFinderOptional<UMaterial> BaseMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlueMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> OrangeMaterial;
		FConstructorStatics()
			//: PlaneMesh(TEXT("/Game/Puzzle/Meshes/PuzzleCube.PuzzleCube"))
			//: PlaneMesh(TEXT("/Game/Geometry/Meshes/1M_Cube"))
			//: PlaneMesh(TEXT("/Game/Geometry/Meshes/cube_1"))
			: PlaneMesh(TEXT("/Game/Geometry/Meshes/blank_cube"))
			, BaseMaterial(TEXT("/Game/Puzzle/Meshes/BaseMaterial.BaseMaterial"))
			, BlueMaterial(TEXT("/Game/Puzzle/Meshes/BlueMaterial.BlueMaterial"))
			, OrangeMaterial(TEXT("/Game/Puzzle/Meshes/OrangeMaterial.OrangeMaterial"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Create static mesh component
	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh0"));
	BlockMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());
	BlockMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));	//turns out the default cube in blender is twice the size of the 1m_cube in unreal
	BlockMesh->SetRelativeLocation(FVector(0.f,0.f,25.f));
	//BlockMesh->SetMaterial(0, ConstructorStatics.BlueMaterial.Get());
	BlockMesh->SetupAttachment(DummyRoot);
	BlockMesh->OnClicked.AddDynamic(this, &AMinesweeper3DBlock::BlockClicked);
	BlockMesh->OnInputTouchBegin.AddDynamic(this, &AMinesweeper3DBlock::OnFingerPressedBlock);

	// Save a pointer to the orange material
	BaseMaterial = ConstructorStatics.BaseMaterial.Get();
	BlueMaterial = ConstructorStatics.BlueMaterial.Get();
	OrangeMaterial = ConstructorStatics.OrangeMaterial.Get();
	ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Flag(TEXT("/Game/Geometry/Meshes/flag_cube"));
	FlagMesh = Flag.Get();
	DefaultMesh = ConstructorStatics.PlaneMesh.Get();
}

void AMinesweeper3DBlock::BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked)
{
	if (OwningGrid->bGameLost || OwningGrid->bGameWon) return;

	if (ButtonClicked.ToString() == "LeftMouseButton") Reveal();
	else Flag();
}

void AMinesweeper3DBlock::OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	Reveal();
}

void AMinesweeper3DBlock::Flag()
{
	if (BlockState == State::flagged)
	{
		BlockState = State::hidden;
		BlockMesh->SetStaticMesh(DefaultMesh);
		OwningGrid->MinesRemaining++;
	}
	else if(BlockState == State::hidden && OwningGrid->MinesRemaining > 0)
	{
		BlockState = State::flagged;
		BlockMesh->SetStaticMesh(FlagMesh);
		OwningGrid->MinesRemaining--;
	}
}

void AMinesweeper3DBlock::Reveal()
{
	if (BlockState == State::revealed || BlockState == State::flagged)	return;

	//Generate mines on the first click so you don't immediately explode
	if (OwningGrid->bFirstClick)
	{
		OwningGrid->FinishSetup(this);
		OwningGrid->bFirstClick = false;
	}

	if (bIsMine)
	{
		if (!OwningGrid->bGameLost)
		{
			OwningGrid->bGameLost = true;
			OwningGrid->RevealMines();
		}
		BlockMesh->SetStaticMesh(OwningGrid->NumberFaces[0]);
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("BOOM!!!!")));
		}*/
		BlockState = State::revealed;
	}
	else
	{
		BlockState = State::revealed;
		BlockMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));
		OwningGrid->BlocksRemaining--;

		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Surrounding Mines: %d"), this->NumSurroundingMines));
		}*/

		if (NumSurroundingMines == 0)
		{
			RevealAdjacentBlocks();
			this->Destroy();
		}
		BlockMesh->SetStaticMesh(OwningGrid->NumberFaces[NumSurroundingMines]);
		OwningGrid->CheckForWin();
	}
}

void AMinesweeper3DBlock::RevealAdjacentBlocks()
{
	for (int x = -1; x < 2; x++)
	{
		for (int y = -1; y < 2; y++)
		{
			for (int z = -1; z < 2; z++)
			{
				if (x == 0 && y == 0 && z == 0)	continue;	//ignore the calling block

				if (OwningGrid->CheckBlockBounds(Xpos + x, Ypos + y, Zpos + z))
				{
					OwningGrid->Blocks[Xpos + x][Ypos + y][Zpos + z]->Reveal();
				}
			}
		}
	}
}

void AMinesweeper3DBlock::Highlight(bool bOn)
{
	// Do not highlight if the block has already been activated.
	/*if (bIsActive)
	{
		return;
	}*/

	if (bOn)
	{
		//BlockMesh->SetMaterial(0, BaseMaterial);
	}
	else
	{
		//BlockMesh->SetMaterial(0, BlueMaterial);
	}
}
