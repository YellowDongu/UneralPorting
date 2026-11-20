// Fill out your copyright notice in the Description page of Project Settings.


#include "StandardMissile.h"
#include "VehicleBase.h"
#include "Radar.h"
#include "ObjectFactory.h"
#include "NiagaraComponent.h"
#include "NiagaraParameterStore.h"
#include "NiagaraFunctionLibrary.h"

AStandardMissile::AStandardMissile()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    MissileID = EWeaponID::Standard;
    FlyDistance = TNumericLimits<float>::Max();
}

void AStandardMissile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (FlyDistance < 0.0f)
    {
        Release();
        return;
    }

    AgilityTimer = FMath::Min(AgilityTimer + DeltaTime, AgilityBonusTime);
    float distance = ConvertFromVelocity(velocity) * DeltaTime;
    FlyDistance -= distance;

	SetActorLocation(GetActorLocation() + GetActorForwardVector() * distance);
    FRotator CurrentRotation = GetActorRotation();

    if (Target == nullptr)
    {
        SetActorRotation(FMath::RInterpTo(CurrentRotation, FRotator(0.0f, CurrentRotation.Yaw, CurrentRotation.Roll), DeltaTime, rotationSpeed));
        return;
    }

    //if (!IsValid(Target.Get()) || Target->IsHidden())
    if (!IsAlive(Target))
    {
        ResetMissileBind();
        return;
    }

    FVector vector = Target->GetActorLocation() - GetActorLocation();
	float sqrMagnitude = vector.SizeSquared();
    //SendDistance(sqrMagnitude);
    GiveRadarWarn.ExecuteIfBound(sqrMagnitude);

    if (sqrMagnitude < ImpactDistance)
    {
        UE_LOG(LogTemp, Log, TEXT("[AStandardMissile::Tick] Collide -> %s"), *Target->GetName());
        GetWorld()->GetSubsystem<UObjectFactory>()->Explosion(GetWorld(), GetActorLocation(), 150.0f, 2.0f);
        Target->HPDamage(damage);

        Release();
    }
    else
        SetActorRotation(FMath::RInterpTo(CurrentRotation, vector.Rotation(), DeltaTime, rotationSpeed * FMath::Lerp(AgilityBonus, 1.0f, AgilityTimer / AgilityBonusTime)));
}

void AStandardMissile::Release()
{
    ResetMissileBind();
    BodyMesh->SetVisibility(false);
    SetActorEnableCollision(false);
    //SetActorHiddenInGame(true);
    SetActorTickEnabled(false);

    
    FTimerHandle timerHandle;
    GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &AStandardMissile::RetriveObject, 5.0f, false);
}
