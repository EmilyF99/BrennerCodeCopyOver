// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Sound/SoundCue.h>
#include "AudioManager.generated.h"

class AFireSpreadPatch;

UCLASS()
class BRIGHTSPARKSPROJECT_API AAudioManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAudioManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Audio Cue Set Up
	UPROPERTY(EditAnywhere)
	UAudioComponent* BackgroundMusic;

	UPROPERTY(EditAnywhere)
	UAudioComponent* WindAmbience;

	UPROPERTY(EditAnywhere)
	UAudioComponent* WinSound;

	UPROPERTY(EditAnywhere)
	UAudioComponent* LoseSound;

	// Cue Callers
	UFUNCTION(BlueprintCallable)
	void PlayWinCue();

	UFUNCTION(BlueprintCallable)
	void PlayLoseCue();

	// Fire Patch Audio Cues
	UPROPERTY()
	TArray<AFireSpreadPatch*> ActiveFirePatches;

	UFUNCTION()
	void RegisterFirePatch(AFireSpreadPatch* Patch);

	UFUNCTION()
	void UpdateFireAudio();

	UPROPERTY()
	APawn* PlayerPawn;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundCue* FireLoopCue;

	UPROPERTY()
	UAudioComponent* FireAudioComponent;
};
