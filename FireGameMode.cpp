
// Fill out your copyright notice in the Description page of Project Settings.


#include "FireGameMode.h"
#include <Kismet/GameplayStatics.h>
#include "FireGamePlayerController.h"
#include "PlayerCharacter.h"
#include "FireGameInstance.h"
#include "AudioManager.h"


AFireGameMode::AFireGameMode()
{
    DefaultPawnClass = nullptr; 
    PlayerControllerClass = AFireGamePlayerController::StaticClass(); 
    PrimaryActorTick.bCanEverTick = true;
}

void AFireGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (UFireGameInstance* GI = Cast<UFireGameInstance>(GetGameInstance()))
    {
        CurrentState = GI->PersistentState;
        UE_LOG(LogTemp, Warning, TEXT("Restored game state from GameInstance: %d"), static_cast<uint8>(CurrentState));
    }

    UE_LOG(LogTemp, Warning, TEXT("GameMode BeginPlay - CurrentState: %d"), static_cast<uint8>(CurrentState));
   

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC && !PC->GetPawn() && DefaultPawnClass)
    {
        APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, FVector::ZeroVector, FRotator::ZeroRotator);
        if (SpawnedPawn)
        {
            PC->Possess(SpawnedPawn);
        }
    }

    if (CurrentState == EGameState::Playing)
    {
        // Start Checking for game over conditions
        GetWorldTimerManager().SetTimer(
            GameOverCheckTimerHandle,
            this,
            &AFireGameMode::CheckGameOverConditions,
            GameOverCheckInterval,
            true
        );

        // Start checking for game win conditions
        GetWorldTimerManager().SetTimer(
            WinGameCheckTimerHandle,
            this,
            &AFireGameMode::CheckGameWinConditions,
            GameWinCheckInterval,
            true
        );
    }
}

void AFireGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Used to count the current level time
    if (CurrentState == EGameState::Playing && !bGameEnded)
    {
        LevelTimer += DeltaSeconds;
    }
}

//  Game Over Conditions
void AFireGameMode::CheckGameOverConditions()
{
    bool bIsOverPercentBurnt = EvaluateBurnPercentage();
    bool bIsSpecialTilesDestroyed = EvaluateSpecialTiles();

    if (bIsOverPercentBurnt)
    {
        if (UFireGameInstance* GI = Cast<UFireGameInstance>(GetGameInstance()))
        {

            GI->FinalEndGameText = EndTextLoseByOverBurn;
        }
    }
    else if (bIsSpecialTilesDestroyed)
    {
        if (UFireGameInstance* GI = Cast<UFireGameInstance>(GetGameInstance()))
        {

            GI->FinalEndGameText = EndTextLoseBySpecialTiles;
        }
    }
    else
    {
        if (UFireGameInstance* GI = Cast<UFireGameInstance>(GetGameInstance()))
        {

            GI->FinalEndGameText = EndTextLoseByHealth;
        }
    }

    if (bIsOverPercentBurnt || bIsSpecialTilesDestroyed)
    {
        GameOver();
    }
}

void AFireGameMode::GameOver()
{
    if (bGameEnded) return;
    bGameEnded = true;
    bPlayerWon = false;
	UE_LOG(LogTemp, Display, TEXT("Game Over Called!"));

    CallGameOverText();

    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAudioManager::StaticClass(), FoundManagers);

    if (FoundManagers.Num() > 0)
    {
        AAudioManager* Manager = Cast<AAudioManager>(FoundManagers[0]);
        if (Manager)
        {
            Manager->PlayLoseCue();
        }
    }

    GetWorldTimerManager().SetTimer(
        CallResultsTimerHandle,
        this,
        &AFireGameMode::CallResultsScreen,
        ResultsScreenTimerInterval,
        true
    );
}

bool AFireGameMode::EvaluateBurnPercentage()
{
    TArray<AActor*> AllFoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFireSpreadPatch::StaticClass(), AllFoundActors);

    TArray<AFireSpreadPatch*> ValidBurnablePatches;
    int32 BurntCount = 0;

    // For all patches found check type
    for (AActor* Actor : AllFoundActors)
    {
        // If the wrong actor is accidentally grabbed then skip it 
        AFireSpreadPatch* Patch = Cast<AFireSpreadPatch>(Actor);
        if (!Patch) continue;

        // Filter out unusable patch types
        switch (Patch->BurnType)
        {
        case ESurfaceBurnType::Quick:
        case ESurfaceBurnType::Slow:
            ValidBurnablePatches.Add(Patch);
            if (Patch->bIsBurnt || Patch->bIsBurning)
                BurntCount++;
            break;

        default:
            break;
        }
    }

    if (ValidBurnablePatches.Num() == 0) return false;
    BurnPercent = BurntCount / static_cast<float>(ValidBurnablePatches.Num());

    // Returns true or false if the burn percent is hire than the set threshold.
    return BurnPercent >= (BurnedThresholdPercent / 100.0f);
}

bool AFireGameMode::EvaluateSpecialTiles()
{
    TArray<AActor*> AllFoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFireSpreadPatch::StaticClass(), AllFoundActors);

    int32 TotalSpecial = 0;
    int32 DestroyedSpecial = 0;

    for (AActor* Actor : AllFoundActors)
    {
        AFireSpreadPatch* Patch = Cast<AFireSpreadPatch>(Actor);

        // Skips all tiles that are not special
        if (!Patch || !Patch->bSpecialTile) continue;

        TotalSpecial++;

        if (Patch->bIsBurning || Patch->bIsBurnt)
        {
            DestroyedSpecial++;
        }
    }

    // Catch for if there was no special tiles
    if (TotalSpecial == 0) return false; 

    return DestroyedSpecial >= TotalSpecial;
}


// Game Win Conditions 
void AFireGameMode::CheckGameWinConditions()
{
    if (bGameEnded) return;

    // Prevent win called immediately after start
    const float MinGameTime = 10.0f; 
    if (GetWorld()->GetTimeSeconds() < MinGameTime) return;

    bool bIsFireCompletelyGone = EvaluateFireExtinguished();

    if (bIsFireCompletelyGone)
    {
        GameWin();
    }
}

void AFireGameMode::GameWin()
{
    if (bGameEnded) return;
    bGameEnded = true;
    bPlayerWon = true;

    UE_LOG(LogTemp, Display, TEXT("Game Win Called!"));


    // Used to set widget before end game
    if (UFireGameInstance* GI = Cast<UFireGameInstance>(GetGameInstance()))
    {
        GI->FinalEndGameText = EndTextWinByBurnout;
    }
    CallGameOverText();

    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAudioManager::StaticClass(), FoundManagers);

    if (FoundManagers.Num() > 0)
    {
        AAudioManager* Manager = Cast<AAudioManager>(FoundManagers[0]);
        if (Manager)
        {
            Manager->PlayWinCue();
        }
    }

    GetWorldTimerManager().SetTimer(
        CallResultsTimerHandle,
        this,
        &AFireGameMode::CallResultsScreen,
        ResultsScreenTimerInterval,
        true
    );
}

bool AFireGameMode::EvaluateFireExtinguished()
{
    TArray<AActor*> AllFoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFireSpreadPatch::StaticClass(), AllFoundActors);

    for (AActor* Actor : AllFoundActors)
    {
        AFireSpreadPatch* Patch = Cast<AFireSpreadPatch>(Actor);
        // If the current item is a patch and if it is burning win has not been met
        if (Patch && Patch->bIsBurning)
        {
            return false; 
        }
    }
    return true; 
}

EGameState AFireGameMode::GetCurrentState() const
{
    return CurrentState;
}

void AFireGameMode::SetGameState(EGameState NewState)
{
    // If the state is already set then exit out 
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    if (UFireGameInstance* GI = Cast<UFireGameInstance>(GetGameInstance()))
    {
        GI->PersistentState = NewState;
    }

    // Tell the controller to update input mode
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        AFireGamePlayerController* FireController = Cast<AFireGamePlayerController>(PC);
        if (FireController)
        {
            FireController->ApplyInputForGameState(NewState);
        }
    }
   
    // Game Controller
    switch (CurrentState)
    {
        case EGameState::Intro:
            UE_LOG(LogTemp, Display, TEXT("Intro State Activated"));
            DefaultPawnClass = nullptr;
            UGameplayStatics::OpenLevel(GetWorld(), IntroLevelName);
            break;

        case EGameState::Playing:
        {
            UE_LOG(LogTemp, Display, TEXT("Playing State Activated"));
            DefaultPawnClass = APlayerCharacter::StaticClass();

            // Showing loading logic
            AFireGamePlayerController* PC = Cast<AFireGamePlayerController>(UGameplayStatics::GetPlayerController(this, 0));
            if (PC)
            {
                PC->ShowLoadingOverlay();
            }
            // Delay before level load so overlay can render
            FTimerHandle LoadTimerHandle;
            GetWorldTimerManager().SetTimer(
                LoadTimerHandle,
                [this]() {
                    UGameplayStatics::OpenLevel(GetWorld(), GameplayLevelName);
                },
                0.5f, // Delay duration in seconds
                false
            );
            break;
        }
        case EGameState::Results:
            UE_LOG(LogTemp, Display, TEXT("Results State Activated"));
            DefaultPawnClass = nullptr;
            FetchEndScreenDetails();
            UGameplayStatics::OpenLevel(GetWorld(), ResultsLevelName);
            break;

        case EGameState::Quitting:
            UE_LOG(LogTemp, Display, TEXT("Quitting Game"));
            // Allows space to add transition / exit messages
            FGenericPlatformMisc::RequestExit(false);
            break;

        default:
            UE_LOG(LogTemp, Display, TEXT("Unknown Game State in use."));
            break;
    }
    
}

void AFireGameMode::CallResultsScreen()
{
    GetWorldTimerManager().ClearTimer(CallResultsTimerHandle);
    SetGameState(EGameState::Results);
}

float AFireGameMode::GetThresholdForRank(EGameRank Rank)
{
    switch (Rank)
    {
    case EGameRank::SS: return 80.0f;
    case EGameRank::S:  return 70.0f;
    case EGameRank::A:  return 60.0f;
    case EGameRank::B:  return 50.0f;
    case EGameRank::C:  return 40.0f;
    case EGameRank::D:  return 0.0f;
    default: return 0.0f;
    }
}

void AFireGameMode::FetchEndScreenDetails()
{
    UE_LOG(LogTemp, Warning, TEXT("Final level time: %.2f seconds"), LevelTimer);
    FireLineCalculatorHandler();
    if (!bPlayerWon)
    {
        // Player lost — assign E rank and a default score
        FinalRank = EGameRank::E;
        SavedPercent = 0; 
    }
    else
    {
        // Force one final update 
        EvaluateBurnPercentage();

        // Fetch the final percent 
        SavedPercent = round(100 - (BurnPercent * 100));

        UE_LOG(LogTemp, Display, TEXT("BurnPercent: %f, SavedPercent: %d"), BurnPercent, SavedPercent);


        // Default to lowest rank
        FinalRank = EGameRank::D;

        // Loop over all the scores, return the highest match
        TArray<EGameRank> RankOrder = {
            EGameRank::SS,
            EGameRank::S,
            EGameRank::A,
            EGameRank::B,
            EGameRank::C,
            EGameRank::D
        };

        for (EGameRank Rank : RankOrder)
        {
            float Threshold = GetThresholdForRank(Rank);
            if (SavedPercent >= Threshold)
            {
                FinalRank = Rank;
                break;
            }
        }
    }

    // Persist this to the Game Instance 
    if (UFireGameInstance* GI = Cast <UFireGameInstance>(GetGameInstance()))
    {
        GI->FinalSavedPercent = SavedPercent;
        GI->FinalSavedRank = FinalRank;
        GI->FinalLevelDuration = LevelTimer;

    }
}

void AFireGameMode::CallGameOverText()
{
    AFireGamePlayerController* PC = Cast<AFireGamePlayerController>(UGameplayStatics::GetPlayerController(this, 0));
    if (PC)
    {
        PC->ShowGameOverText();
    }
}

void AFireGameMode::FireLineCalculatorHandler()
{
    /* There is a large amount of operations needed to calculate the amount of lines dug by the player,
        therefore for tidier code this is moved into a handler */

    // Global Line Counter
    int LineCount = 0;

    // Collecting all dug patches
    TArray<AFireSpreadPatch*> DugPatches = GetAllDugPatches();
    UE_LOG(LogTemp, Warning, TEXT("Collected %d dug patches for fire line evaluation."), DugPatches.Num());

    if (DugPatches.Num() == 0)
	{
        // If there are no dug patches then lines are persisted as 0
        LineCount = 0;
	}
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Starting line calculation for %d dug patches."), DugPatches.Num());
        LineCount = LineCalculator(DugPatches, LineCount);
        UE_LOG(LogTemp, Warning, TEXT("Lines found %d."), LineCount);
    }

    if (UFireGameInstance* GI = Cast <UFireGameInstance>(GetGameInstance()))
    {
        GI->FinalLineCount = LineCount;
    }
}
int AFireGameMode::LineCalculator(TArray<AFireSpreadPatch*> DugPatches, int LineCount)
{
    for (int32 i = DugPatches.Num() - 1; i >= 0; --i)
    {
        AFireSpreadPatch* CurrentPatch = DugPatches[i];
        if (!CurrentPatch) continue;

        bool bHasValidNeighbor = false;

        UE_LOG(LogTemp, Warning, TEXT("Checking patch %s for valid neighbors..."), *CurrentPatch->GetName());
        for (AFireSpreadPatch* Neighbor : CurrentPatch->AdjacentPatches)
        {
            if (!Neighbor || Neighbor == CurrentPatch) continue;
            if (!Neighbor->bIsDug) continue;

            UE_LOG(LogTemp, Warning, TEXT("Patch %s at (%f, %f) — Neighbor %s at (%f, %f)"),
                *CurrentPatch->GetName(),
                CurrentPatch->GetActorLocation().X, CurrentPatch->GetActorLocation().Y,
                *Neighbor->GetName(),
                Neighbor->GetActorLocation().X, Neighbor->GetActorLocation().Y);

            ECompass Dir = GetDirectionBetween(CurrentPatch, Neighbor);
            UE_LOG(LogTemp, Warning, TEXT("Neighbor %s classified as direction: %s"),
                *Neighbor->GetName(),
                *UEnum::GetValueAsString(Dir));

            // Only count horizontal or vertical neighbors
            if (Dir == ECompass::North || Dir == ECompass::South ||
                Dir == ECompass::East || Dir == ECompass::West)
            {
                bHasValidNeighbor = true;
                break;
            }
        }

        // If no valid neighbors, remove this patch from the list
        if (!bHasValidNeighbor)
        {
            UE_LOG(LogTemp, Warning, TEXT("No neighbours found for patch %s — removing from list."), *CurrentPatch->GetName());
            DugPatches.RemoveAt(i);

        }
    }

    // Find the neighbours for the current patch
    for (AFireSpreadPatch* Patch : DugPatches)
    {
      if (!Patch || Patch->bVisitedForLineCheck) continue;

      for (ECompass Dir : {ECompass::East, ECompass::North})
      {
          TArray<AFireSpreadPatch*> Line;

          TraceLine(Patch, Dir, Line);
          TraceLine(Patch, GetOppositeDirection(Dir), Line);

          if (Line.Num() >= 3)
          {
            LineCount++;
            UE_LOG(LogTemp, Warning, TEXT("Found line of %d patches starting at %s in direction %s"), Line.Num(), *Patch->GetName(), *UEnum::GetValueAsString(Dir));
             
            for (AFireSpreadPatch* LinePatch : Line)
            {
                LinePatch->bVisitedForLineCheck = true;
            }
          }
      }
    }
    //UE_LOG(LogTemp, Warning, TEXT("Line calculation complete. Remaining patches: %d"), DugPatches.Num());
    return LineCount;
}


ECompass  AFireGameMode::GetDirectionBetween(AFireSpreadPatch* From, AFireSpreadPatch* To)
{
    // Threshold to account for manual tile placement
    float Threshold = 10.0f;

    FVector Delta = To->GetActorLocation() - From->GetActorLocation();

    bool bIsHorizontal = FMath::Abs(Delta.X) > Threshold && FMath::Abs(Delta.Y) < Threshold;
    bool bIsVertical = FMath::Abs(Delta.Y) > Threshold && FMath::Abs(Delta.X) < Threshold;

    if (bIsVertical)
    {
        return Delta.Y > 0 ? ECompass::North : ECompass::South;
    }
    else if (bIsHorizontal)
    {
        return Delta.X > 0 ? ECompass::East : ECompass::West;
    }

    return ECompass::None;
}

TArray<AFireSpreadPatch*> AFireGameMode::GetAllDugPatches()
{
    TArray<AFireSpreadPatch*> DugPatches;

    TArray<AActor*> EndGameAllPatches;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFireSpreadPatch::StaticClass(), EndGameAllPatches);

    for (AActor* Actor : EndGameAllPatches)
    {
        AFireSpreadPatch* Patch = Cast<AFireSpreadPatch>(Actor);
        if (Patch && Patch->bIsDug)
        {
            DugPatches.Add(Patch);
        }
    }

    return DugPatches;
}

AFireSpreadPatch* AFireGameMode::GetNeighborInDirection(AFireSpreadPatch* From, ECompass Direction)
{
    for (AFireSpreadPatch* Neighbor : From->AdjacentPatches)
    {
        if (!Neighbor || !Neighbor->bIsDug) continue;

        ECompass Dir = GetDirectionBetween(From, Neighbor);
        if (Dir == Direction)
        {
            return Neighbor;
        }
    }
    return nullptr;
}

ECompass AFireGameMode::GetOppositeDirection(ECompass Dir)
{
    switch (Dir)
    {
    case ECompass::North: return ECompass::South;
    case ECompass::South: return ECompass::North;
    case ECompass::East:  return ECompass::West;
    case ECompass::West:  return ECompass::East;
    default: return ECompass::None;
    }
}

void AFireGameMode::TraceLine(AFireSpreadPatch* StartPatch, ECompass Direction, TArray<AFireSpreadPatch*>& OutLine)
{
    AFireSpreadPatch* Current = StartPatch;

    while (Current)
    {
        if (Current->bVisitedForLineCheck) break;

        OutLine.Add(Current);

        AFireSpreadPatch* Next = GetNeighborInDirection(Current, Direction);
        if (!Next || !Next->bIsDug || Next->bVisitedForLineCheck)
            break;

        Current = Next;
    }
}
