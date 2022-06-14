// Copyright Epic Games, Inc. All Rights Reserved.

#include "Minesweeper3DBlockGrid.h"
#include "Minesweeper3DBlock.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/UnrealString.h"

#define LOCTEXT_NAMESPACE "PuzzleBlockGrid"

AMinesweeper3DBlockGrid::AMinesweeper3DBlockGrid()
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Set defaults
	BlockSpacing = 100.f;	//size of the 1M_Cube in unreal units
	theta = 3.14f / 4;
	phi = -3.14f / 4;

	//menu widgets
	ConstructorHelpers::FClassFinder<UUserWidget> Settings(TEXT("/Game/Geometry/Meshes/Settings.Settings_C"));
	ConstructorHelpers::FClassFinder<UUserWidget> HUD(TEXT("/Game/Geometry/Meshes/HUD.HUD_C"));
	ConstructorHelpers::FClassFinder<UUserWidget> CustomSettings(TEXT("/Game/Geometry/Meshes/CustomGame.CustomGame_C"));
	if (Settings.Class) SettingsWidget = Settings.Class;
	if (HUD.Class) HUDWidget = HUD.Class;
	if (CustomSettings.Class) CustomSettingsWidget = CustomSettings.Class;

	//Get materials for block faces
	ConstructorHelpers::FObjectFinderOptional<UStaticMesh> mine(TEXT("/Game/Geometry/Meshes/mine_cube"));
	NumberFaces.Add(mine.Get());

	for (int i = 1; i <= 26; i++)
	{
		FString path = "/Game/Geometry/Meshes/";
		path.Append(FString::FromInt(i));
		path.Append("_cube");
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> cube(*path);
		NumberFaces.Add(cube.Get());
	}

}

void AMinesweeper3DBlockGrid::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//if(!bIsFreeCam)	
	UpdateCameraPosition();
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Yellow, FString::Printf(TEXT("X: %f, Y: %f, Z: %f, Radius: %f"),
			Camera->GetActorForwardVector().X, Camera->GetActorForwardVector().Y, Camera->GetActorForwardVector().Z, DistanceFromCenter()));
	}*/
}

void AMinesweeper3DBlockGrid::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	InputComponent->BindAction("ZoomIn", IE_Pressed, this, &AMinesweeper3DBlockGrid::ZoomIn);
	InputComponent->BindAction("ZoomOut", IE_Pressed, this, &AMinesweeper3DBlockGrid::ZoomOut);
	InputComponent->BindAction("ZoomIn", IE_Released, this, &AMinesweeper3DBlockGrid::ZoomStop);
	InputComponent->BindAction("ZoomOut", IE_Released, this, &AMinesweeper3DBlockGrid::ZoomStop);
	InputComponent->BindAction("SettingsMenu", IE_Pressed, this, &AMinesweeper3DBlockGrid::ToggleSettings);
	InputComponent->BindAction("FreeCam", IE_Pressed, this, &AMinesweeper3DBlockGrid::EnableMousePanning);
	InputComponent->BindAction("FreeCam", IE_Released, this, &AMinesweeper3DBlockGrid::DisableMousePanning);
	InputComponent->BindAction("Reset", IE_Pressed, this, &AMinesweeper3DBlockGrid::StartGame);

	
	InputComponent->BindAxis("LeftRight", this, &AMinesweeper3DBlockGrid::MoveLeftRight);
	InputComponent->BindAxis("UpDown", this, &AMinesweeper3DBlockGrid::MoveUpDown);
	InputComponent->BindAxis("InOut", this, &AMinesweeper3DBlockGrid::MoveInOut);
	InputComponent->BindAxis("FreeCamYaw", this, &AMinesweeper3DBlockGrid::ChangeTheta);
	InputComponent->BindAxis("FreeCamPitch", this, &AMinesweeper3DBlockGrid::ChangePhi);

}

void AMinesweeper3DBlockGrid::BeginPlay()
{
	Super::BeginPlay();
	bIsSettings = false;
	ToggleSettings();

	//Find the camera in the world and set its starting position
	//Presumably, adding additional cameras will bork this up.
	TArray<AActor*> ActorsToFind;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), ActorsToFind);
	Camera = ActorsToFind[0];
	ResetCameraPosition();

	UGameplayStatics::GetPlayerController(this, 0)->SetViewTarget(Camera);
	
}

/*------------- Interface -------------*/

void AMinesweeper3DBlockGrid::ToggleSettings()
{
	if (!bIsSettings)
	{
		if (UClass* SettingsWidgetClass = SettingsWidget.Get())
		{
			ActiveWidget = CreateWidget<UUserWidget>(GetWorld(), SettingsWidgetClass);
			ActiveWidget->AddToViewport(0);
			bIsSettings = true;
		}
	}
	else
	{
		CloseMenu();
	}
}

void AMinesweeper3DBlockGrid::ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (CurrentWidget != nullptr)
	{
		CloseMenu();
	}

	if (NewWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), NewWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

//Close the menu via the exit button
void AMinesweeper3DBlockGrid::CloseMenu()
{
	ActiveWidget->RemoveFromViewport();
	if (SecondaryWidget)	SecondaryWidget->RemoveFromViewport();
	
	if (UClass* HUDWidgetClass = HUDWidget.Get())
	{
		ActiveWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		ActiveWidget->AddToViewport(0);
		bIsSettings = false;
	}

	if (bFirstGame)
	{
		StartGame();
		bFirstGame = false;
	}
}

void AMinesweeper3DBlockGrid::GetActiveCheckbox(UCheckBox* box)
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, box->GetName());
	}*/

	FString Name = box->GetName();

	if (Name == "DifficultyBoxBeginner")
	{
		NewSize = 5;
		NumMines = NewSize * NewSize * NewSize * MinesPercentage;
		if (SecondaryWidget)	SecondaryWidget->RemoveFromViewport();
	}
	else if (Name == "DifficultyBoxIntermediate")
	{
		NewSize = 8;
		NumMines = NewSize * NewSize * NewSize * MinesPercentage;
		if (SecondaryWidget)	SecondaryWidget->RemoveFromViewport();
	}
	else if (Name == "DifficultyBoxExpert")
	{
		NewSize = 11;
		NumMines = NewSize * NewSize * NewSize * MinesPercentage;
		if (SecondaryWidget)	SecondaryWidget->RemoveFromViewport();
	}
	else if (Name == "DifficultyBoxCustom")
	{
		NewSize = 1;
		if (UClass* CustomSettingsWidgetClass = CustomSettingsWidget.Get())
		{
			SecondaryWidget = CreateWidget<UUserWidget>(GetWorld(), CustomSettingsWidgetClass);
			SecondaryWidget->AddToViewport(0);
		}
	}
}

void AMinesweeper3DBlockGrid::ChangeSize(FString Size_in)
{
	NewSize = FCString::Atoi(*Size_in);
}

void AMinesweeper3DBlockGrid::ChangeMines(FString NewMines)
{
	float Mines_in = FCString::Atof(*NewMines);
	//If user is unputing an integer numer of mines, convert to percentage
	if (Mines_in >= 1.f)
	{
		NumMines = Mines_in;
	}
	else if (Mines_in >= 0 && Mines_in < 1.f)
	{
		NumMines = Size * Size * Size * Mines_in;
	}

}

/*---------- Input -----------*/
void AMinesweeper3DBlockGrid::EnableMousePanning()
{
	UGameplayStatics::GetPlayerController(this, 0)->bShowMouseCursor = false;
	UGameplayStatics::GetPlayerController(this, 0)->SetInputMode(FInputModeGameOnly());
}

void AMinesweeper3DBlockGrid::DisableMousePanning()
{

	UGameplayStatics::GetPlayerController(this, 0)->bShowMouseCursor = true;
	UGameplayStatics::GetPlayerController(this, 0)->SetInputMode(FInputModeGameAndUI());
	
}

void AMinesweeper3DBlockGrid::MoveLeftRight(float AxisValue)
{
	TranslationInputDirection += Camera->GetActorRightVector() * -AxisValue;
}

void AMinesweeper3DBlockGrid::MoveUpDown(float AxisValue)
{
	TranslationInputDirection += Camera->GetActorUpVector() * AxisValue;
}
void AMinesweeper3DBlockGrid::MoveInOut(float AxisValue)
{
	TranslationInputDirection += Camera->GetActorForwardVector() * AxisValue;
}

//Negative AxisValue 
void AMinesweeper3DBlockGrid::ZoomIn()
{
	ZoomCamera(true);
}

//Positive AxisValue
void AMinesweeper3DBlockGrid::ZoomOut()
{
	ZoomCamera(false);
}

void AMinesweeper3DBlockGrid::ChangeTheta(float AxisVal)
{
	theta += 0.035f * AxisVal;
}

void AMinesweeper3DBlockGrid::ChangePhi(float AxisVal)
{
	phi += 0.035f * AxisVal;
}

void AMinesweeper3DBlockGrid::ZoomStop(){}

void AMinesweeper3DBlockGrid::ZoomCamera(bool bZoomOut)
{
	float delta = ZoomSpeed;
	if (bZoomOut)	delta *= -2;

	radius += delta;
}

void AMinesweeper3DBlockGrid::ResetCameraPosition()
{
	FVector CameraLoc(-1000.f, -1000.f, 1000.f + (Size * 100.f));
	theta = 3.14f / 4;
	phi = -3.14f / 4;
	Camera->SetActorLocation(CameraLoc);
}

void AMinesweeper3DBlockGrid::UpdateCameraPosition()
{
	//Wrap theta between 0 and 2pi (as defined by the spherical coordinate system)
	if (theta < 0)	theta += 6.28;
	if (theta > 6.28)	theta -= 6.28;

	if (phi < -3.14)	phi += 6.28;
	if (phi > 3.14)	phi -= 6.28;

	TranslationInputDirection.Normalize();
	TranslationInputDirection *= 10.f;
	TranslationInputDirection += Camera->GetActorLocation();
	Camera->SetActorLocation(TranslationInputDirection);

	//Not sure why the equation for pitch is so weird, I basically just messed around until I found something that worked. Avoids gimbal lock tho.
	FRotator rotation((-phi * 180.f / 3.14f) - 90.f, theta * 180.f / 3.14f, 0.0f);
	Camera->SetActorRotation(rotation);
	
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.02, FColor::Yellow, FString::Printf(TEXT("Theta: %f, Phi: %f, Pitch: %f, Yaw: %f, Roll: %f"),
			theta, phi, rotation.Pitch, rotation.Yaw, rotation.Roll));
	}*/

	TranslationInputDirection = FVector(0, 0, 0);
}

/*----------- Level Generation ------------*/

void AMinesweeper3DBlockGrid::StartGame()
{
	//needed to finish generation when the first block is clicked
	bFirstClick = true;
	bGameLost = false;
	bGameWon = false;
	DestroyBlocks();
	Size = NewSize;
	CubeCenter.X = 100.f * (Size - 1) * 0.5;
	CubeCenter.Y = 100.f * (Size - 1) * 0.5;
	CubeCenter.Z = 100.f * (Size - 1) * 0.5;
	radius = 150.f * Size;
	GenerateBlocks();
	ResetCameraPosition();

	ElapsedTime = 0;
	MinesRemaining = NumMines;
	BlocksRemaining = Size * Size * Size;
	GetWorldTimerManager().ClearTimer(GameClockTimer);
}

//Spawn the 3D cube of blocks and link them to the BlockGrid
void AMinesweeper3DBlockGrid::GenerateBlocks()
{
	//reserve 1st dimension (surely there has to be a better way to initialize a 3d array, but the constructor list is lacking)
	Blocks.Empty();
	BlockList.Empty();
	Blocks.Reserve(Size);
	for (int i = 0; i < Size; i++)
	{
		Blocks.Emplace();
		Blocks[i].Reserve(Size);
	}

	//reserve 2nd dimension
	for (int i = 0; i < Size; i++)
	{
		for (int j = 0; j < Size; j++)
		{
			Blocks[i].Emplace();
			Blocks[i][j].Reserve(Size);
		}
	}

	//reserve 3rd dimension
	for (int i = 0; i < Size; i++)
	{
		for (int j = 0; j < Size; j++)
		{
			for (int k = 0; k < Size; k++)
			{
				Blocks[i][j].Emplace();
			}
		}
	}

	//Spawn each block
	for (int Xpos = 0; Xpos < Size; Xpos++)
	{
		for (int Ypos = 0; Ypos < Size; Ypos++)
		{
			for (int Zpos = 0; Zpos < Size; Zpos++)
			{
				//Make position vector, offset from grid location
				const FVector BlockLocation = FVector(Xpos * BlockSpacing, Ypos * BlockSpacing, Zpos * BlockSpacing); //+ GetActorLocation();

				//Spawn a block
				AMinesweeper3DBlock* NewBlock = GetWorld()->SpawnActor<AMinesweeper3DBlock>(BlockLocation, FRotator(0, 0, 0));

				if (NewBlock != nullptr)
				{
					NewBlock->OwningGrid = this;
					Blocks[Xpos][Ypos][Zpos] = NewBlock;
					BlockList.Add(NewBlock);
					NewBlock->Xpos = Xpos;
					NewBlock->Ypos = Ypos;
					NewBlock->Zpos = Zpos;
					NewBlock->bIsRevealed = false;
				}
			}
		}
	}
}

//Determine which blocks are mines
void AMinesweeper3DBlockGrid::GenerateMines()
{
	//Randomize mines
	for (int i = 0; i < 5000; i++)
	{
		BlockList.Swap(FMath::RandRange(0, BlockList.Num() - 1), FMath::RandRange(0, BlockList.Num() - 1));
	}

	for (int i = 0; i < NumMines; i++)
	{
		//Ignore the blocks surrounding the first click so more blocks are revealed when the game starts
		if (BlockList[i]->bGenerationSafelock)
		{
			//Keep the first NumMines entries in BlockList as all mines so they're easy to find in RevealMines()
			BlockList.RemoveAt(i);
			i--;
		}
		else
		{
			BlockList[i]->bIsMine = true;
		}
	}
}

//Find number of surrounding mines for each block
void AMinesweeper3DBlockGrid::AssignSurroundingMineTotals()
{
	for (int i = 0; i < Blocks.Num(); i++)
	{
		for (int j = 0; j < Blocks[0].Num(); j++)
		{
			for (int k = 0; k < Blocks[0][0].Num(); k++)
			{
				if (!Blocks[i][j][k]->bIsMine)
				{
					Blocks[i][j][k]->NumSurroundingMines = CalcSurroundingMines(i, j, k);
				}
			}
		}
	}
}

//Called when the first block is clicked
void AMinesweeper3DBlockGrid::FinishSetup(AMinesweeper3DBlock *block)
{
	SafelockBlocks(block);
	GenerateMines();
	AssignSurroundingMineTotals();

	GetWorldTimerManager().SetTimer(GameClockTimer, this, &AMinesweeper3DBlockGrid::AdvanceTimer, 1.0f, true);
}

void AMinesweeper3DBlockGrid::DestroyBlocks()
{
	for (int i = 0; i < Blocks.Num(); i++)
	{
		for (int j = 0; j < Blocks.Num(); j++)
		{
			for (int k = 0; k < Blocks.Num(); k++)
			{
				//Sometimes this pointer can be valid, but the underlying object isn't so we can't just check the pointer
				if(Blocks[i][j][k]->IsValidLowLevel()) Blocks[i][j][k]->Destroy();
			}
		}
	}
}

/*---------- Utility ----------*/

//Sets a flag for all blocks surrounding block to make sure they don't become mines
//this gives the player more information when they start the game
void AMinesweeper3DBlockGrid::SafelockBlocks(AMinesweeper3DBlock* block)
{
	int Xpos = block->Xpos;
	int Ypos = block->Ypos;
	int Zpos = block->Zpos;

	for (int x = -1; x < 2; x++)
	{
		for (int y = -1; y < 2; y++)
		{
			for (int z = -1; z < 2; z++)
			{
				if (CheckBlockBounds(Xpos + x, Ypos + y, Zpos + z))
				{
					Blocks[Xpos + x][Ypos + y][Zpos + z]->bGenerationSafelock = true;
				}
			}
		}
	}
}

//Called on game loss
void AMinesweeper3DBlockGrid::RevealMines()
{
	for (int i = 0; i < NumMines; i++)
	{
		BlockList[i]->Reveal();
	}
	GetWorldTimerManager().ClearTimer(GameClockTimer);
}

void AMinesweeper3DBlockGrid::CheckForWin()
{
	bGameWon = BlocksRemaining == NumMines;
	if(bGameWon)	GetWorldTimerManager().ClearTimer(GameClockTimer);
}

//When checking surrounding blocks, ensure they are not outside the bounds of the Blocks 3d array
bool AMinesweeper3DBlockGrid::CheckBlockBounds(int Xpos, int Ypos, int Zpos)
{
	return Xpos >= 0 && Xpos < Size && Ypos >= 0 && Ypos < Size&& Zpos >= 0 && Zpos < Size;
}

//Check the 26 blocks surrounding a block at a given x, y, and z coordinate to determine how many mines surround it
int AMinesweeper3DBlockGrid::CalcSurroundingMines(int Xpos, int Ypos, int Zpos)
{
	int AdjacentMines = 0;
	
	for (int x = -1; x < 2; x++)
	{
		for (int y = -1; y < 2; y++)
		{
			for (int z = -1; z < 2; z++)
			{
				if (CheckBlockBounds(Xpos + x, Ypos + y, Zpos + z))
				{
					if (Blocks[Xpos + x][Ypos + y][Zpos + z]->bIsMine)
					{
						AdjacentMines++;
					}
				}
			}
		}
	}
	return AdjacentMines;
}

float AMinesweeper3DBlockGrid::DistanceFromCenter()
{
	float X = Camera->GetActorLocation().X - CubeCenter.X;
	float Y = Camera->GetActorLocation().Y - CubeCenter.Y;
	float Z = Camera->GetActorLocation().Z - CubeCenter.Z;
	return FMath::Sqrt(X * X + Y * Y + Z * Z);
}

void AMinesweeper3DBlockGrid::AdvanceTimer()
{
	ElapsedTime++;
}

#undef LOCTEXT_NAMESPACE
