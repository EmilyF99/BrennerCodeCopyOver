// Fill out your copyright notice in the Description page of Project Settings.

#include "FireSpreadPatch.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "AudioManager.h"
#include "Kismet/GameplayStatics.h"
#include <NiagaraFunctionLibrary.h>

// Sets default values
AFireSpreadPatch::AFireSpreadPatch()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    //Setting up plane collision
    DetectionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionVolume"));
    SetRootComponent(DetectionVolume);
    DetectionVolume->SetBoxExtent(FVector(80.f));
    DetectionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionVolume->SetCollisionObjectType(ECC_WorldDynamic);
    DetectionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

// Called when the game starts or when spawned
void AFireSpreadPatch::BeginPlay()
{
    Super::BeginPlay();

    // Register Audio Manager
    AudioManager = Cast<AAudioManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAudioManager::StaticClass()));
    if (AudioManager)
    {
        AudioManager->RegisterFirePatch(this);
    }

    // Needed for finding the neighbouring ground patches
    TArray<AActor*> FoundActors;
    float SearchRadius = 80.f;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));

    UKismetSystemLibrary::SphereOverlapActors(
        this,
        GetActorLocation(),
        SearchRadius,
        ObjectTypes,
        AFireSpreadPatch::StaticClass(),
        TArray<AActor*>(),
        FoundActors
    );

    // Iterate over all the actors found, if they are a patch, add to a list of patches
    for (AActor* Actor : FoundActors)
    {
        AFireSpreadPatch* Patch = Cast<AFireSpreadPatch>(Actor);
        if (Patch && Patch != this && !AdjacentPatches.Contains(Patch))
        {
            AdjacentPatches.Add(Patch);
        }
    }

    if (BurnType == ESurfaceBurnType::Burnt)
    {
        SetUpBurntPatches();
    }

    // Debug print
    /*
    UE_LOG(LogTemp, Warning, TEXT("[%s] found %d neighbors:"), *GetName(), AdjacentPatches.Num());
    for (AFireSpreadPatch* Neighbor : AdjacentPatches)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Neighbor->GetName());
    }
    */
}

// Called every frame
void AFireSpreadPatch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFireSpreadPatch::Ignite(bool bInstantSpread)
{
    if (bIsBurning || bIsBurnt || bIsDug || BurnType == ESurfaceBurnType::NonBurnable)
        return;
    
    bIsBurning = true;
    //UE_LOG(LogTemp, Warning, TEXT("%s has caught fire"), *GetName());

    if (FireEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            FireEffect,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }

    // Add Fire Sound
    if (AudioManager)
    {
        AudioManager->UpdateFireAudio();
    }

    // Timer that calls burn out when it is completed
    GetWorldTimerManager().SetTimer(
        BurnOutTimerHandle,
        this,
        &AFireSpreadPatch::BurnOut,
        BurnDuration,
        false
    );

    float SpreadDelay = FetchSpreadDelay();
    // Once on fire try to spread to it's neighbour
    UE_LOG(LogTemp, Warning, TEXT("%s is calling SpreadFire()..."), *GetName());
    GetWorld()->GetTimerManager().SetTimer(
        SpreadTimerHandle,
        this,
        &AFireSpreadPatch::SpreadFire,
        SpreadDelay, 
        false
    );
}

void AFireSpreadPatch::SpreadFire()
{

    // From the list of neighbours, only look for the valid neighbours
    TArray<AFireSpreadPatch*> ValidNeighbors;
    for (AFireSpreadPatch* Neighbor : AdjacentPatches)
    {
        if (Neighbor && !Neighbor->bIsBurning && !Neighbor->bIsBurnt && !Neighbor->bPendingIgnition && !Neighbor->bIsDug)
        {
            ValidNeighbors.Add(Neighbor);
        }
    }

    // If there are valid neigbours to ignite select between one and max number to burn
    int32 NumToSpread = FMath::Min(3, ValidNeighbors.Num());

    for (int32 i = 0; i < NumToSpread; ++i)
    {
        int32 Index = FMath::RandRange(0, ValidNeighbors.Num() - 1);
        AFireSpreadPatch* Target = ValidNeighbors[Index];

      //  Target->bPendingIgnition = true;
        Target->Ignite();

        ValidNeighbors.RemoveAt(Index); 
    }
}

void AFireSpreadPatch::SetUpBurntPatches()
{
    bIsBurnt = true;
}

float AFireSpreadPatch::FetchSpreadDelay()
{
    // Based on the type, set the spread delay
    float fSpreadDelay = 0.0f;

    switch (BurnType)
    {
    case ESurfaceBurnType::Quick:
        fSpreadDelay = 2.f;
        break;
    case ESurfaceBurnType::Slow:
        fSpreadDelay = 5.f;
        break;
    default:
        break;
    }
    return fSpreadDelay * FMath::RandRange(MinSpreadDelay, MaxSpreadDelay);
}

void AFireSpreadPatch::BurnOut()
{ 
    bIsBurning = false;
    bIsBurnt = true;

    TArray<USceneComponent*> SelectedComponent;
    GetRootComponent()->GetChildrenComponents(true, SelectedComponent);
    for (USceneComponent* Component : SelectedComponent)
    {
        UNiagaraComponent* NiagaraComp = Cast<UNiagaraComponent>(Component);
        if (NiagaraComp)
        {
            NiagaraComp->Deactivate(); 
        }
    }

    // Stop Sound
    if (AudioManager)
    {
        AudioManager->UpdateFireAudio();
    }
    UE_LOG(LogTemp, Warning, TEXT("%s fire burned out, marked as burnt"), *GetName());

    // Call to Engine Implemented Function
    OnPatchBurnt();
}
