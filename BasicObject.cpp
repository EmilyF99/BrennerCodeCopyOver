// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicObject.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "FireSpreadPatch.h"


// Sets default values
ABasicObject::ABasicObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creating the Collision Sphere, needed for local spread
	FireSpreadSphere = CreateDefaultSubobject<USphereComponent>(TEXT("FireSpreadSphere"));
	FireSpreadSphere->SetupAttachment(RootComponent);
	FireSpreadSphere->SetSphereRadius(SpreadRadius);
	FireSpreadSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FireSpreadSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Overlaps with everything, may need to be changed
	FireSpreadSphere->SetCollisionResponseToAllChannels(ECR_Overlap);

}

// Called when the game starts or when spawned
void ABasicObject::BeginPlay()
{
	Super::BeginPlay();

	// Needed for starting fires 
	if (bIsOnFire)
	{
		StartFire();
	}

	// Check ground type
	CastRayToDetectGround();
}

// Called every frame
void ABasicObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO: Replace with a Timer
	if (bIsOnFire)
	{
		SpreadFireNearestObject();
		CastRayToDetectGround();
	}

}

void ABasicObject::StartFire()
{
		bIsOnFire = true;
		//UE_LOG(LogTemp, Warning, TEXT("%s has caught fire!"), *GetName());

		// Spawn fire effect
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
}

void ABasicObject::SpreadFireNearestObject()
{
	if (bIsOnFire)
	{
		// Check if overlap in range
		TArray<AActor*> OverlappingActors;
		FireSpreadSphere->GetOverlappingActors(OverlappingActors);
		
		for (AActor* OverlappingActor : OverlappingActors)
		{
			// If there is overlap check if next object is marked as flammable
			ABasicObject* NextObject = Cast<ABasicObject>(OverlappingActor);
			if (NextObject && NextObject->bIsFlammable && !NextObject->bIsOnFire)
			{	
				/* Create Fire Delay Timer 
				*  This stops the fire from instant combustion 
				*  Args:
				*	(float) FlammabilityFactor: Main delay timer, inputted as seconds 
				*	(float) MinSpreadDelay: Used in Rand to deviate from FlamabilityFactor
				*   (float) MaxSpreadDelay: Used in Rand to deviate from FlamabilityFactor
				*/
				float SpreadDelay = FlammabilityFactor * FMath::RandRange(MinSpreadDelay, MaxSpreadDelay);
				FTimerHandle TimerHandle;
				FTimerDelegate TimerDel;

				// Binds delay to the set fire function
				TimerDel.BindUFunction(this, FName("SetNextObjectOnFire"), NextObject);
				GetWorldTimerManager().SetTimer(TimerHandle, TimerDel, SpreadDelay, false);
			}
		}

	}
}

void ABasicObject::SetNextObjectOnFire(ABasicObject* TargetObject)
{
	if (TargetObject && TargetObject->bIsFlammable && !TargetObject->bIsOnFire)
	{
		TargetObject->StartFire();
		UE_LOG(LogTemp, Warning, TEXT("Object spreading fire to: %s (%s)"),
			*TargetObject->GetName(), *TargetObject->GetClass()->GetName());

	}
}

void ABasicObject::CastRayToDetectGround()
{
	// Setting RayCast Start and End Point
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 100.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); 

	// Setting Up Line Tracer
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{	
		if (bDrawDebugLine)
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f, 0, 2.0f);
			UE_LOG(LogTemp, Warning, TEXT("Ray hit actor: %s at location: %s"), *Hit.GetActor()->GetName(), *Hit.ImpactPoint.ToString());
		}

		// Check if we hit a floor patch
		AFireSpreadPatch* Patch = Cast<AFireSpreadPatch>(Hit.GetActor());
		if (Patch && bIsOnFire)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Hit surface patch: %s"), *Patch->GetName());
			if (!Patch->bIsBurnt && !Patch->bIsBurning && !Patch->bIsDug)
			{
				Patch->Ignite();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No valid floor patch under %s"), *GetName());
		}
	}
}
