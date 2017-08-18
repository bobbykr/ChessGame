// Fill out your copyright notice in the Description page of Project Settings.

#include "ChessRook.h"
#include "ChessGameGameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"

AChessRook::AChessRook()
{
	PrimaryActorTick.bCanEverTick = true;

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		UE_LOG(LogTemp, Error, TEXT("Don't need to load mesh if im a server!"));
	}
	else
	{
		bool AllSuccessfull = true;
		const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshLoader(TEXT("StaticMesh'/Game/Rook.Rook'"));

		if (!MeshLoader.Succeeded())
			AllSuccessfull = false;

		if (AllSuccessfull)
		{
			Mesh = MeshLoader.Object;
		}
	}
}

void AChessRook::BeginPlay()
{
	Super::BeginPlay();


	AllowedMoves.Add(FVector2D(0, 7));
	AllowedMoves.Add(FVector2D(0, -7));
	AllowedMoves.Add(FVector2D(7, 0));
	AllowedMoves.Add(FVector2D(-7, 0));

	UStaticMeshComponent* MeshComponent = GetStaticMeshComponent();
	MeshComponent->SetStaticMesh(Mesh);

	switch (type)
	{
	case 0:
		MaterialToUse = UMaterialInstanceDynamic::Create(WhiteMaterial, this);
		MaterialToUse->SetScalarParameterValue("Emissive", 0);
		break;
	case 1:
		MaterialToUse = UMaterialInstanceDynamic::Create(BlackMaterial, this);
		MaterialToUse->SetScalarParameterValue("Emissive", 0);
		break;
	default:
		MaterialToUse = UMaterialInstanceDynamic::Create(WhiteMaterial, this);
		UE_LOG(LogTemp, Warning, TEXT("Color on pawn not compatible!"));
		break;
	}


	MeshComponent->SetMaterial(0, MaterialToUse);
	// Make Object Movable
	MeshComponent->SetMobility(EComponentMobility::Movable);

	// Make Object Clickable
	AChessGameGameModeBase* GameMode = Cast<AChessGameGameModeBase>(GetWorld()->GetAuthGameMode());

	GameMode->ClickableActors.Add(this);
}

void AChessRook::GetPossibleMoveHighlight(TArray<int>& indexes)
{
	indexes.Empty();
	for (int i = 0; i < AllowedMoves.Num(); i++)
	{
		// Highlight all squares in the direction
		for (int y = 1; y < FMath::Abs(AllowedMoves[i].Y) + 1; y++)
		{
			int NextI = CurrentSlotI + FMath::Sign<float>(AllowedMoves[i].Y)*y;
			int NextJ = CurrentSlotJ;

			AChessPiece* ChessPiece = Cast<AChessPiece>(GetActorFromSlot(NextI, NextJ));

			if (NextI >= 0 && NextI < 8 && NextJ >= 0 && NextJ < 8)
			{
				if (ChessPiece == nullptr)
				{
					indexes.Add(ToOneDimentional(NextI, NextJ));
				}
				else if (ChessPiece->type == this->type)
				{
					y = FMath::Abs(AllowedMoves[i].Y) + 1; // Exit loop
				}
				else if (ChessPiece->type != this->type)
				{
					indexes.Add(ToOneDimentional(NextI, NextJ));
					y = FMath::Abs(AllowedMoves[i].Y) + 1; // Exit loop
				}
			}
		}

		for (int x = 1; x < FMath::Abs(AllowedMoves[i].X) + 1; x++)
		{
			int NextI = CurrentSlotI;
			int NextJ = CurrentSlotJ + FMath::Sign<float>(AllowedMoves[i].X)*x;

			AChessPiece* ChessPiece = Cast<AChessPiece>(GetActorFromSlot(NextI, NextJ));

			if (NextI >= 0 && NextI < 8 && NextJ >= 0 && NextJ < 8)
			{
				if (ChessPiece == nullptr)
				{
					indexes.Add(ToOneDimentional(NextI, NextJ));
				}
				else if (ChessPiece->type == this->type)
				{
					x = FMath::Abs(AllowedMoves[i].X) + 1; // Exit loop
				}
				else if (ChessPiece->type != this->type)
				{
					indexes.Add(ToOneDimentional(NextI, NextJ));
					x = FMath::Abs(AllowedMoves[i].X) + 1; // Exit loop
				}
			}
		}
	}
}

void AChessRook::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction & ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	Position = GetPositionFromSlot(CurrentSlotI, CurrentSlotJ);

	SetActorLocation(Position);
}

bool AChessRook::isValidMove(int IndexToMoveToI, int IndexToMoveToJ)
{
	int DeltaI = IndexToMoveToI - CurrentSlotI;
	int DeltaJ = IndexToMoveToJ - CurrentSlotJ;
	bool UpDown = FMath::Abs<int>(DeltaI);
	bool LeftRight = FMath::Abs<int>(DeltaJ);
	bool StraightLine = !(LeftRight && UpDown);

	if (StraightLine)
	{
		// Check if the place you are moving towards has any collisions along the way...
		if (UpDown)
		{
			if (FMath::Sign<int>(DeltaI) == 1)
			{
				// Check every position up to IndexToMoveToI
				for (int y = 1; y <= DeltaI; y++)
				{
					int NextI = CurrentSlotI + y;
					int NextJ = CurrentSlotJ;

					AChessPiece* ChessPiece = Cast<AChessPiece>(GetActorFromSlot(NextI, NextJ));

					if (ChessPiece != nullptr)
					{
						UE_LOG(LogTemp, Warning, TEXT("Found a chesspiece at I: %s J: %s"), *FString::FromInt(NextI), *FString::FromInt(NextJ));
						if (ChessPiece->type == this->type)
						{
							return false;
						}
						else
						{
							if (NextI != IndexToMoveToI || NextJ != IndexToMoveToJ)
								return false;
						}
					}
				}
			}
			else if (FMath::Sign<int>(DeltaI) == -1)
			{
				// Check every position down to IndexToMoveToJ
				for (int y = -1; y >= DeltaI; y--)
				{
					int NextI = CurrentSlotI + y;
					int NextJ = CurrentSlotJ;

					AChessPiece* ChessPiece = Cast<AChessPiece>(GetActorFromSlot(NextI, NextJ));

					if (ChessPiece != nullptr)
					{
						if (ChessPiece->type == this->type)
						{
							return false;
						}
						else
						{
							if (NextI != IndexToMoveToI || NextJ != IndexToMoveToJ)
								return false;
						}
					}
				}
			}
		} 
		else if (LeftRight)
		{
			if (FMath::Sign<int>(DeltaJ) == 1)
			{
				for (int x = 1; x <= DeltaJ; x++)
				{
					int NextI = CurrentSlotI;
					int NextJ = CurrentSlotJ + x;

					AChessPiece* ChessPiece = Cast<AChessPiece>(GetActorFromSlot(NextI, NextJ));

					if (ChessPiece != nullptr)
					{
						if (ChessPiece->type == this->type)
						{
							return false;
						}
						else
						{
							if (NextI != IndexToMoveToI || NextJ != IndexToMoveToJ)
								return false;
						}
					}
				}
			}
			else if (FMath::Sign<int>(DeltaJ) == -1)
			{
				for (int x = -1; x >= DeltaJ; x--)
				{
					int NextI = CurrentSlotI;
					int NextJ = CurrentSlotJ + x;

					AChessPiece* ChessPiece = Cast<AChessPiece>(GetActorFromSlot(NextI, NextJ));

					if (ChessPiece != nullptr)
					{
						if (ChessPiece->type == this->type)
						{
							return false;
						}
						else
						{
							if (NextI != IndexToMoveToI || NextJ != IndexToMoveToJ)
								return false;
						}
					}
				}
			}
		}


		return true;
	}
	else
	{
		return false;
	}
}
