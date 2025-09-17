// Fill out your copyright notice in the Description page of Project Settings.
#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "FireGameMode.h"
#include <Kismet/GameplayStatics.h>


// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	// Create and attach the camera
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(RootComponent); 
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Defining the hit to be returned
	FHitResult GroundHit;
		
	// Only Trace the ground if the relevant tool is here.
	switch (CurrentTool)
	{
	case EToolType::Shovel: 
		TargetedGround = TraceGround(PlayerReachDistanceShovel, GroundHit);
		break;

	case EToolType::DripTorch: 
		TargetedGround = TraceGround(PlayerReachDistanceDripTorch, GroundHit);
		break;
	default:
		// No Tool Found
		break;
	}
}


AFireSpreadPatch* APlayerCharacter::TraceGround(float TraceDistance, FHitResult& OutHit)
{
	/* 
		Get the current Camera Pitch on the Z-Axis.
		If the aim is too high, do not progress with casting.
	*/
	float CameraPitchZ = FirstPersonCameraComponent->GetForwardVector().Z;
	if (CameraPitchZ > MinimumAimPitch)
	{
		TargetedGround = nullptr;
		return nullptr; 
	}
	else
	{
		FVector Start = FirstPersonCameraComponent->GetComponentLocation();
		FVector Direction = FirstPersonCameraComponent->GetForwardVector();
		Direction.Z -= PitchAngleBias;
		Direction.Normalize();

		FVector End = Start + Direction * TraceDistance;

		// Generating a ray cast
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			OutHit,
			Start,
			End,
			ECC_Visibility,
			Params
		);
		if (bIsDebugRayOn)
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 2.0f);
		}

		// When there is a hit return the ground type hit
		if (bHit)
		{
			AFireSpreadPatch* Patch = Cast<AFireSpreadPatch>(OutHit.GetActor());
			if (Patch)
			{
				return Patch;
			}
			else
			{
				UE_LOG(LogTemp, Display, TEXT("Hit actor is not a valid floor patch: %s"), *OutHit.GetActor()->GetName());
			}
		}

		return nullptr;
	}
	
}

void APlayerCharacter::TryUseTool(EToolType ToolType, TFunction<void(AFireSpreadPatch*)> ToolAction, const FString& ActionName)
{

	// Checking for cool down
	if (GetWorld()->GetTimerManager().IsTimerActive(CooldownHandle))
	{
		UE_LOG(LogTemp, Display, TEXT("%s is on cooldown"), *ActionName);
		return;
	}

	// If there is no cool down then move onto try and use the tool
	if (CurrentTool == ToolType && TargetedGround)
	{
		if (!TargetedGround->bIsBurning && !TargetedGround->bIsBurnt && !TargetedGround->bIsDug)
		{
			ToolAction(TargetedGround);

			float Delay = ReuseDelay;
			GetWorld()->GetTimerManager().SetTimer(CooldownHandle, Delay, false);
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Cannot %s this patch: %s"), *ActionName.ToLower(), *TargetedGround->GetName());
		}
	}
}

void APlayerCharacter::TryShovel()
{
	TryUseTool(EToolType::Shovel, [](AFireSpreadPatch* Patch)
		{
			Patch->bIsDug = true;
			Patch->BurnType = ESurfaceBurnType::Dug;
			UE_LOG(LogTemp, Display, TEXT("Dug patch: %s"), *Patch->GetName());
		}, TEXT("Dug"));
}

void APlayerCharacter::TryDripTorch()
{
	TryUseTool(EToolType::DripTorch, [](AFireSpreadPatch* Patch)
		{
			Patch->Ignite();
			UE_LOG(LogTemp, Display, TEXT("Ignited patch: %s"), *Patch->GetName());
		}, TEXT("Ignited"));
}

