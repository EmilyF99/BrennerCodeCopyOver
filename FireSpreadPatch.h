// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Components/BoxComponent.h>
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "FireSpreadPatch.generated.h"

class AAudioManager;

UENUM(BlueprintType)
enum class ESurfaceBurnType : uint8
{
	Quick     UMETA(DisplayName = "Quick"),
	Slow      UMETA(DisplayName = "Slow"),
	NonBurnable UMETA(DisplayName = "Non-Burnable"),
	Burnt     UMETA(DisplayName = "Already Burnt"),
	Dug       UMETA(DisplayName = "Dug")
};

UCLASS()
class BRIGHTSPARKSPROJECT_API AFireSpreadPatch : public AActor
{
	GENERATED_BODY()
	
public:	

	// Sets default values for this actor's properties
	AFireSpreadPatch();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* DetectionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireSpread")
	TArray<AFireSpreadPatch*> AdjacentPatches;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	bool bIsBurning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	bool bIsBurnt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	bool bIsDug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	bool bPendingIgnition = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	bool bSpecialTile = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	bool bVisitedForLineCheck = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	UNiagaraSystem* FireEffect;

	void Ignite(bool bInstantSpread = false);

	UFUNCTION(BlueprintCallable, Category = "Fire Ground")
	void SpreadFire();

	UFUNCTION(BlueprintCallable, Category = "Fire Ground")
	void SetUpBurntPatches();

	UFUNCTION(BlueprintCallable, Category = "Fire Ground")
	float FetchSpreadDelay();

	UFUNCTION(BlueprintCallable, Category = "Fire Ground")
	void BurnOut();

	UFUNCTION(BlueprintImplementableEvent, Category = "Visuals")
	void OnPatchBurnt();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	float MinSpreadDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	float MaxSpreadDelay = 15.0f;

	UPROPERTY(EditAnywhere, Category = "FireSpread")
	float BurnDuration = 45.0f;


	// Timer Handles
	FTimerHandle SpreadTimerHandle;
	FTimerHandle BurnOutTimerHandle;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Ground")
	ESurfaceBurnType BurnType = ESurfaceBurnType::Quick;

	UPROPERTY()
	AAudioManager* AudioManager;



};
