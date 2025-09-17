// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FireGameMode.h"
#include "FireGamePlayerController.generated.h"


UCLASS()
class BRIGHTSPARKSPROJECT_API AFireGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void BeginPlay();
	void ApplyInputForGameState(EGameState GameState);

	// Loading Wdiget
	void ShowLoadingOverlay();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	TSubclassOf<UUserWidget> LoadingOverlayClass;

	UPROPERTY()
	UUserWidget* ActiveLoadingWidget;

	// Game Over Text Widget
	UFUNCTION(Category = "UI")
	void ShowGameOverText();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> GameOverMessageWidgetClass;

	UPROPERTY()
	UUserWidget* ActiveGameOverWidget;
};
