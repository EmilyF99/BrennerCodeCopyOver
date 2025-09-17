// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FireSpreadPatch.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "FireGameMode.h"
#include "PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class EToolType : uint8
{
	NoTool     UMETA(DisplayName = "NoToolSelected"),
	Shovel     UMETA(DisplayName = "Shovel"),
	DripTorch  UMETA(DisplayName = "DripTorch"),
};

UCLASS()
class BRIGHTSPARKSPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Raycast")
	float PlayerReachDistanceShovel = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Raycast")
	float PlayerReachDistanceDripTorch = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Raycast")
	bool bIsDebugRayOn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Raycast")
	EToolType CurrentTool = EToolType::NoTool;

	UFUNCTION(BlueprintCallable, Category = "Tool Raycast")
	AFireSpreadPatch* TraceGround(float TraceDistance, FHitResult& OutHit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Raycast", meta = (AllowPrivateAccess = "true"))
	AFireSpreadPatch* TargetedGround;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Raycast", meta = (ToolTip = "Minimum downward aim to interact (Z of forward vector, range -1 to 1)"))
	float MinimumAimPitch = -0.2f;

	UPROPERTY(EditAnywhere, Category = "Tool Raycast")
	float PitchAngleBias = 0.3f;
	
	// Tool Triggers
	UFUNCTION(BlueprintCallable, Category = "Tool Raycast")
	void TryShovel();

	UFUNCTION(BlueprintCallable, Category = "Tool Raycast")
	void TryDripTorch();

	void TryUseTool(EToolType ToolType, TFunction<void(AFireSpreadPatch*)> ToolAction, const FString& ActionName);

	// Tool Reuse Setup
	FTimerHandle CooldownHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Raycast")
	float ReuseDelay = 2.0f;
};
