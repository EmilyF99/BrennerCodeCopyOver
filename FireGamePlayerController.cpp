// Fill out your copyright notice in the Description page of Project Settings.

#include "FireGamePlayerController.h"
#include "Blueprint/UserWidget.h"
#include <Kismet/GameplayStatics.h>

void AFireGamePlayerController::BeginPlay()
{

    Super::BeginPlay();

    if (AFireGameMode* GM = Cast<AFireGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        UE_LOG(LogTemp, Warning, TEXT("Game state at BeginPlay: %d"), GM->GetCurrentState());
        ApplyInputForGameState(GM->GetCurrentState());
    }

}

void AFireGamePlayerController::ApplyInputForGameState(EGameState GameState)
{
    UE_LOG(LogTemp, Warning, TEXT("Player Controller: %s"), *GetNameSafe(this));

    switch (GameState)
    {
    case EGameState::Intro:
    case EGameState::Results:
        bShowMouseCursor = true;
        SetShowMouseCursor(true);
        SetInputMode(FInputModeUIOnly());

        if (GetPawn())
        {
            DisableInput(this);
        }
        break;

    case EGameState::Playing:
        bShowMouseCursor = false;
        SetShowMouseCursor(false);
        SetInputMode(FInputModeGameOnly());

        if (GetPawn())
        {
            EnableInput(this);
        }
        break;

    default:
        break;
    }
}

void AFireGamePlayerController::ShowLoadingOverlay()
{
    if (LoadingOverlayClass)
    {
        UUserWidget* LoadingOverlay = CreateWidget<UUserWidget>(this, LoadingOverlayClass);
        if (LoadingOverlay)
        {
            UE_LOG(LogTemp, Warning, TEXT("Inside loading overlay"));
            LoadingOverlay->AddToViewport(100);
            ActiveLoadingWidget = LoadingOverlay;
        }
    }
}

void AFireGamePlayerController::ShowGameOverText()
{
    if (GameOverMessageWidgetClass)
    {
        UUserWidget* GameOverTextOverlay = CreateWidget<UUserWidget>(this, GameOverMessageWidgetClass);
        if (GameOverTextOverlay)
        {
            UE_LOG(LogTemp, Warning, TEXT("Inside GameOverText Overlay"));
            GameOverTextOverlay->AddToViewport(100);
            ActiveLoadingWidget = GameOverTextOverlay;
        }
    }
}

