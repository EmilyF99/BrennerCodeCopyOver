// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "BasicObject.generated.h"


UCLASS()
class BRIGHTSPARKSPROJECT_API ABasicObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasicObject();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Nearest Object")
	class USphereComponent* FireSpreadSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Nearest Object")
	float SpreadRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Nearest Object")
	bool bIsFlammable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Nearest Object")
	bool bIsOnFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Nearest Object")
	bool bDrawDebugLine = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Nearest Object")
	float FlammabilityFactor = 1.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Nearest Object")
	float MinSpreadDelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Nearest Object")
	float MaxSpreadDelay = 2.0f;

	UFUNCTION(BlueprintCallable, Category = "Fire Nearest Object")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Fire Nearest Object")
	void SpreadFireNearestObject();

	UFUNCTION(BlueprintCallable, Category = "Fire Nearest Object")
	void SetNextObjectOnFire(ABasicObject* TargetObject);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Nearest Object")
	UNiagaraSystem* FireEffect;

	UFUNCTION(BlueprintCallable, Category = "Fire Spreading")
	void CastRayToDetectGround();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
