// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FireSpreadPatch.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FireGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameRank : uint8
{
	SS,
	S,
	A,
	B,
	C,
	D, 
	E // Only used for game lose
};

UENUM(BlueprintType, meta = (ScriptName = "BrightSparksGameStateEnum"))
enum class EGameState : uint8
{
	Intro,
	Playing,
	Results,
	Quitting
};

// Used for getting the final dug line count
UENUM(BlueprintType)
enum class ECompass : uint8
{
	None,
	North,
	East,
	South,
	West
};

UCLASS()
class BRIGHTSPARKSPROJECT_API AFireGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFireGameMode();
	virtual void BeginPlay() override;
	void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Control")
	float BurnedThresholdPercent = 70.f;

	UFUNCTION(BlueprintCallable, Category = "Game Over")
	void CheckGameOverConditions();

	UFUNCTION(BlueprintCallable, Category = "Game Over")
	void GameOver();

	UFUNCTION(BlueprintCallable, Category = "Game Over")
	bool EvaluateBurnPercentage();

	UFUNCTION(BlueprintCallable, Category = "Game Over")
	bool EvaluateSpecialTiles();

	TArray<AFireSpreadPatch*> AllPatches;

	UFUNCTION(BlueprintCallable, Category = "Game Win")
	void CheckGameWinConditions();

	UFUNCTION(BlueprintCallable, Category = "Game Win")
	void GameWin();

	UFUNCTION(BlueprintCallable, Category = "Game Win")
	bool EvaluateFireExtinguished();

	EGameState GetCurrentState() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Control")
	bool bGameEnded = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Control")
	float  BurnPercent = 0.0f;

	// Setup for the game over timer 
	FTimerHandle GameOverCheckTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Control")
	float GameOverCheckInterval = 2.0f;

	// Setup for the game win timer 
	FTimerHandle WinGameCheckTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Control")
	float GameWinCheckInterval = 2.0f;

	// Setup for the Results Screen Timer
	FTimerHandle CallResultsTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Control")
	float ResultsScreenTimerInterval = 5.0f;

	// Game State Logic
	UPROPERTY(BlueprintReadOnly, Category = "Game State")
	EGameState CurrentState = EGameState::Intro;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetGameState(EGameState NewState);

	void CallResultsScreen();

	// Level Set Ups 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	FName IntroLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	FName GameplayLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	FName ResultsLevelName;

	// Game Rank Code 
	UFUNCTION(BlueprintCallable, Category = "Game Rank")
	float GetThresholdForRank(EGameRank Rank);

	UFUNCTION(BlueprintCallable, Category = "Game Rank")
	void FetchEndScreenDetails();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Rank")
	int  SavedPercent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Rank")
	EGameRank FinalRank;

	UPROPERTY(BlueprintReadOnly, Category = "Game Rank")
	bool bPlayerWon = false;

	UPROPERTY(BlueprintReadOnly, Category = "Game Rank")
	float LevelTimer = 0.0f;

	// End Text (Loads before results screen)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Endgame Text")
	FText EndTextWinByBurnout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Endgame Text")
	FText EndTextLoseByHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Endgame Text")
	FText EndTextLoseBySpecialTiles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Endgame Text")
	FText EndTextLoseByOverBurn;

	UFUNCTION()
	void CallGameOverText();

	// METHODS FOR FIRE LINE CALCULATOR
	UFUNCTION()
	void FireLineCalculatorHandler();

	UFUNCTION()
	TArray<AFireSpreadPatch*> GetAllDugPatches();

	UFUNCTION()
	int LineCalculator(TArray<AFireSpreadPatch*> DugPatches, int LineCount);

	UFUNCTION()
	ECompass GetDirectionBetween(AFireSpreadPatch* From, AFireSpreadPatch* To);

	UFUNCTION()
	AFireSpreadPatch* GetNeighborInDirection(AFireSpreadPatch* From, ECompass Direction);

	UFUNCTION()
	ECompass GetOppositeDirection(ECompass Dir);

	UFUNCTION()
	void TraceLine(AFireSpreadPatch* StartPatch, ECompass Direction, TArray<AFireSpreadPatch*>& OutLine);



};

