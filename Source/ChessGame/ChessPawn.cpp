// Fill out your copyright notice in the Description page of Project Settings.

#include "ChessPawn.h"
#include "ChessGameGameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"

AChessPawn::AChessPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	bool AllSuccessfull = true;
	const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshLoader(TEXT("StaticMesh'/Game/Bonde.Bonde'"));
	
	if (!MeshLoader.Succeeded())
		AllSuccessfull = false;

	if (AllSuccessfull)
	{
		Mesh = MeshLoader.Object;
	}
}

void AChessPawn::BeginPlay()
{
	Super::BeginPlay();

	if (!type)
	{
		AllowedMoves.Add(FVector2D(0, 1));
		AllowedMoves.Add(FVector2D(1, 1));
		AllowedMoves.Add(FVector2D(-1, 1));
	}
	else
	{
		AllowedMoves.Add(FVector2D(0, -1));
		AllowedMoves.Add(FVector2D(1, -1));
		AllowedMoves.Add(FVector2D(-1, -1));
	}

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

void AChessPawn::GetPossibleMoveHighlight(TArray<int>& indexes)
{
	indexes.Empty();
	for (int i = 0; i < AllowedMoves.Num(); i++)
	{
		int NextI = CurrentSlotI + AllowedMoves[i].Y;
		int NextJ = CurrentSlotJ + AllowedMoves[i].X;

		if (NextI >= 0 && NextI < 8 && NextJ >= 0 && NextJ < 8)
		{
			// Check if theres a piece occupying the slot already
			
			AChessPiece* ChessPiece = Cast<AChessPiece>(GetActorFromSlot(NextI, NextJ));

			if(!isPieceInSlot(NextI, NextJ) && NextI - CurrentSlotI != 0 && NextJ - CurrentSlotJ == 0)
				indexes.Add(ToOneDimentional(NextI, NextJ));
			else if (ChessPiece != nullptr)
			{
				if (ChessPiece->type != this->type && NextI - CurrentSlotI != 0 && NextJ - CurrentSlotJ != 0)
				{
					indexes.Add(ToOneDimentional(NextI, NextJ));
				}
			}	
		}
	}
}

void AChessPawn::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction & ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	Position = GetPositionFromSlot(CurrentSlotI, CurrentSlotJ);

	SetActorLocation(Position);
}

bool AChessPawn::isValidMove(int IndexToMoveToI, int IndexToMoveToJ)
{
	bool MoveAllowed = false;
	for (int i = 0; i < AllowedMoves.Num(); i++)
	{
		float DeltaY = IndexToMoveToI - CurrentSlotI;
		float DeltaX = IndexToMoveToJ - CurrentSlotJ;

		UE_LOG(LogTemp, Error, TEXT("DeltaX: %s"), *FString::SanitizeFloat(DeltaX));
		UE_LOG(LogTemp, Error, TEXT("DeltaY: %s"), *FString::SanitizeFloat(DeltaY));


		if (DeltaY == AllowedMoves[i].Y && DeltaX == AllowedMoves[i].X)
		{
			AChessPiece* ChessPiece = Cast<AChessPiece>(GetActorFromSlot(IndexToMoveToI, IndexToMoveToJ));

			if (!isPieceInSlot(IndexToMoveToI, IndexToMoveToJ) && IndexToMoveToI - CurrentSlotI != 0 && IndexToMoveToJ - CurrentSlotJ == 0)
				MoveAllowed = true;
			else if (ChessPiece != nullptr)
			{
				if (ChessPiece->type != this->type && IndexToMoveToI - CurrentSlotI != 0 && IndexToMoveToJ - CurrentSlotJ != 0)
				{
					MoveAllowed = true;
				}
			}
		}
	}

	return MoveAllowed;
}
