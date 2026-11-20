// Fill out your copyright notice in the Description page of Project Settings.


#include "FourAirtoAirMissile.h"

#include "VehicleBase.h"
#include "Radar.h"
#include "ObjectFactory.h"
#include "NiagaraComponent.h"
#include "NiagaraParameterStore.h"
#include "NiagaraFunctionLibrary.h"

AFourAirtoAirMissile::AFourAirtoAirMissile()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    MissileID = EWeaponID::FAAM;
    FlyDistance = TNumericLimits<float>::Max();
}

void AFourAirtoAirMissile::Tick(float DeltaTime)
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

    if (!Target.IsValid())
    {
        FRotator CurrentRotation = GetActorRotation();
        SetActorRotation(FMath::RInterpTo(CurrentRotation, FRotator(0.0f, CurrentRotation.Yaw, CurrentRotation.Roll), DeltaTime, rotationSpeed));
        return;
    }

    if (Target->IsHidden())
    {
        Target = nullptr;
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
        AVehicleBase* vehicle = Cast<AVehicleBase>(Target.Get());
        if (vehicle != nullptr)
            vehicle->HPDamage(damage);
        Release();
    }
    else
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), vector.GetSafeNormal().Rotation(), DeltaTime, rotationSpeed * FMath::Lerp(AgilityBonus, 1.0f, AgilityTimer / AgilityBonusTime)));
}

void AFourAirtoAirMissile::Release()
{
    ResetMissileBind();

    SetActorEnableCollision(false);
    //SetActorHiddenInGame(true);
    SetActorTickEnabled(false);
    BodyMesh->SetVisibility(false);

    FTimerHandle timerHandle;
    GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &AFourAirtoAirMissile::RetriveObject, 5.0f, false);
}
