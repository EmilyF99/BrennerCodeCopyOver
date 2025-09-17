// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioManager.h"
#include "FireSpreadPatch.h"
#include <Kismet/GameplayStatics.h>
#include "Components/AudioComponent.h"



// Sets default values
AAudioManager::AAudioManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Setting up audio components
	BackgroundMusic = CreateDefaultSubobject<UAudioComponent>(TEXT("BackgroundMusic"));
	BackgroundMusic->SetupAttachment(RootComponent);

	WindAmbience = CreateDefaultSubobject<UAudioComponent>(TEXT("BackgroundAmbience"));
	WindAmbience->SetupAttachment(RootComponent);

	WinSound = CreateDefaultSubobject<UAudioComponent>(TEXT("WinCue"));
	WinSound->SetupAttachment(RootComponent);

	LoseSound = CreateDefaultSubobject<UAudioComponent>(TEXT("LoseCue"));
	LoseSound->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AAudioManager::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// Start ambient noise and background music
	if (BackgroundMusic) BackgroundMusic->Play();
	if (WindAmbience) WindAmbience->Play();
	
}

// Called every frame
void AAudioManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerPawn) return;

	int32 NearbyBurningCount = 0;

	for (AFireSpreadPatch* Patch : ActiveFirePatches)
	{
		if (Patch && Patch->bIsBurning)
		{
			float Distance = FVector::Dist(Patch->GetActorLocation(), PlayerPawn->GetActorLocation());
			if (Distance < 1000.0f)
			{
				++NearbyBurningCount;
			}
		}
	}

	if (NearbyBurningCount > 0)
	{
		if (!FireAudioComponent || !FireAudioComponent->IsPlaying())
		{
			FireAudioComponent = UGameplayStatics::SpawnSound2D(this, FireLoopCue);
		}

		if (FireAudioComponent)
		{
			float TargetVolume = FMath::Clamp(NearbyBurningCount / 5.0f, 0.2f, 1.0f);
			float CurrentVolume = FireAudioComponent->VolumeMultiplier;
			float SmoothedVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, 2.0f);
			FireAudioComponent->SetVolumeMultiplier(SmoothedVolume);
		}
	}
	else
	{
		if (FireAudioComponent && FireAudioComponent->IsPlaying())
		{
			FireAudioComponent->FadeOut(1.0f, 0.0f);
		}
	}

}

// End Game Cues
void AAudioManager::PlayWinCue()
{
	if (WinSound && !WinSound->IsPlaying()) WinSound->Play();
}

void AAudioManager::PlayLoseCue()
{
	if (LoseSound && !LoseSound->IsPlaying()) LoseSound->Play();
}

void AAudioManager::RegisterFirePatch(AFireSpreadPatch* Patch)
{
	if (Patch && !ActiveFirePatches.Contains(Patch))
	{
		ActiveFirePatches.Add(Patch);
	}
}

void AAudioManager::UpdateFireAudio()
{
	int32 BurningCount = 0;

	for (AFireSpreadPatch* Patch : ActiveFirePatches)
	{
		if (Patch && Patch->bIsBurning)
		{
			++BurningCount;
		}
	}

	if (BurningCount > 0)
	{
		if (!FireAudioComponent || !FireAudioComponent->IsPlaying())
		{
			FireAudioComponent = UGameplayStatics::SpawnSound2D(this, FireLoopCue);
		}

		if (FireAudioComponent)
		{
			float Volume = FMath::Clamp(BurningCount / 5.0f, 0.2f, 1.0f);
			FireAudioComponent->SetVolumeMultiplier(Volume);
		}
	}
	else
	{
		if (FireAudioComponent && FireAudioComponent->IsPlaying())
		{
			FireAudioComponent->FadeOut(1.0f, 0.0f);
		}
	}
}