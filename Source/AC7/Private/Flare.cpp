// Fill out your copyright notice in the Description page of Project Settings.

#include "Flare.h"
#include "ObjectFactory.h"
//#include "NiagaraSystem.h"
//#include "Defined.h"
//#include "NiagaraParameterStore.h"

AFlare::AFlare()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bTickEvenWhenPaused = false;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    SetActorEnableCollision(false);
}

void AFlare::PostInitProperties()
{
	Super::PostInitProperties();
    SetActorEnableCollision(false);
}

void AFlare::PostInitializeComponents()
{
	Super::PostInitializeComponents();
    Body = Cast<UNiagaraComponent>(GetRootComponent());
    Trail = Cast<UNiagaraComponent>(Body->GetAttachChildren()[0]);

    FNiagaraVariable DirectionParam(FNiagaraTypeDefinition::GetVec3Def(), TEXT("User.MoveVector"));
    FlareBodyDirection.Init(Body->GetOverrideParameters(), DirectionParam);
}

void AFlare::BeginPlay()
{
	Super::BeginPlay();

    ShootSpeed = 330.0;
    CurrentLifeTime = LifeTime;
}

void AFlare::Tick(float DeltaTime)
{
    constexpr float falseGravity = 980.0 * 0.25;
	Super::Tick(DeltaTime);
    CurrentLifeTime -= DeltaTime;
    if (CurrentLifeTime <= 0.0f)
    {
        Disappear();
        return;
    }

    //CurrentSpeed = FMath::Lerp(ShootSpeed, MinimumSpeed, 1.0f - CurrentLifeTime / LifeTime);
    //FVector Direction = GetActorForwardVector();
    //SetActorLocation(GetActorLocation() + Direction * CurrentSpeed * DeltaTime);
    //FlareBodyDirection.SetValue(FVector3f(Direction));
    
    //CurrentVelocity.Z += GetWorld()->GetGravityZ() * DeltaTime;
    CurrentVelocity.Z += -falseGravity * DeltaTime;

    CurrentSpeed = CurrentVelocity.Size();
    float TargetSpeed = FMath::Max(CurrentSpeed, falseGravity);
    CurrentVelocity = CurrentVelocity.GetSafeNormal() * FMath::FInterpTo(CurrentSpeed, falseGravity, DeltaTime, 0.5f);
    SetActorLocation(GetActorLocation() + CurrentVelocity * DeltaTime);

    FlareBodyDirection.SetValue(FVector3f(CurrentVelocity.GetSafeNormal()));
}

void AFlare::Disappear()
{
    //Body->SetVisibility(false);
    SetActorTickEnabled(false);
    SetActorEnableCollision(false); 
    FTimerHandle dummyHandle;
    GetWorld()->GetTimerManager().SetTimer(dummyHandle, this, &AFlare::RetriveObject, LifeTime, false);
}

void AFlare::RetriveObject()
{
    if (GetWorld() == nullptr)
    {
        Destroy();
        return;
    }

    UObjectFactory* objectFactory = GetWorld()->GetSubsystem<UObjectFactory>();
    if (objectFactory != nullptr)
    {
        SetActorEnableCollision(false);
        SetActorHiddenInGame(true);
        SetActorTickEnabled(false);
        objectFactory->Release(this);
    }
    else
        Destroy();
}

void AFlare::Shoot(USceneComponent* flarePod, const FVector& DirectionVector)
{
	const FTransform& WorldTransform = flarePod->GetComponentTransform();
	SetActorTransform(WorldTransform);

    //double velocity = FMath::Max(DirectionVector.Length(), 1.0);
    SetActorEnableCollision(true);
    SetActorHiddenInGame(false);
    SetActorTickEnabled(true);
    Body->SetVisibility(true);

	CurrentVelocity = FMath::VRandCone(GetActorForwardVector(), FMath::DegreesToRadians(15.0f)) * ConvertFromVelocity(50.0);

    if (!DirectionVector.IsZero())
    {
        CurrentVelocity += DirectionVector;
        SetActorRotation(CurrentVelocity.Rotation().Quaternion());
        ShootSpeed = DirectionVector.Length();
    }

    CurrentLifeTime = LifeTime;
	HP = TNumericLimits<int>::Max();
}