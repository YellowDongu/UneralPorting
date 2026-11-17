// Fill out your copyright notice in the Description page of Project Settings.


#include "SemiActiveAntiAirMissile.h"
#include "Aircraft.h"
#include "Radar.h"
#include "FireControlSystem.h"
#include "ObjectFactory.h"
#include "NiagaraComponent.h"
#include "NiagaraParameterStore.h"
#include "NiagaraFunctionLibrary.h"


ASemiActiveAntiAirMissile::ASemiActiveAntiAirMissile()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    MissileID = EWeaponID::SAAM;
    FlyDistance = TNumericLimits<float>::Max();
}

void ASemiActiveAntiAirMissile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (FlyDistance < 0.0f)
    {
        Release();
        return;
    }


    float distance = ConvertFromVelocity(velocity) * DeltaTime;
    FlyDistance -= distance;
    FVector position = GetActorLocation();

    SetActorLocation(position + GetActorForwardVector() * distance);

    if (!IsAlive(Owner) || !FCSInfo->IsEmpty())
        return;

    Rebind((*FCSInfo)[0].Target);

    if (!IsAlive(Target) || !(*FCSInfo)[0].IsLock())
        return;
    {
        //FRotator CurrentRotation = GetActorRotation();
        //SetActorRotation(FMath::RInterpTo(CurrentRotation, FRotator(0.0f, CurrentRotation.Yaw, CurrentRotation.Roll), DeltaTime, rotationSpeed));
    }

    FVector vector = Target->GetActorLocation() - position;
    float sqrMagnitude = vector.SizeSquared();
    GiveRadarWarn.ExecuteIfBound(sqrMagnitude);

    if (sqrMagnitude < ImpactDistance)
    {
        UE_LOG(LogTemp, Log, TEXT("[AStandardMissile::Tick] Collide -> %s"), *Target->GetName());

        GetWorld()->GetSubsystem<UObjectFactory>()->Explosion(GetWorld(), position, 150.0f, 2.0f);
        AVehicleBase* vehicle = Cast<AVehicleBase>(Target.Get());
        if (vehicle != nullptr)
            vehicle->HPDamage(damage);

        Release();
    }
    else
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), vector.Rotation(), DeltaTime, rotationSpeed));
}

void ASemiActiveAntiAirMissile::Rebind(TWeakObjectPtr<AVehicleBase> newTarget)
{
    if (Target == newTarget)
        return;

    Target.Reset();
    GiveRadarWarn.Unbind();
    if (TargetRader.IsValid())
        TargetRader->UnTrace(this);
    TargetRader.Reset();

    if (!IsAlive(newTarget))
        return;

    Target = newTarget;
    TargetRader = newTarget->FindComponentByClass<URadar>();

    if (TargetRader != nullptr)
    {
        GiveRadarWarn.BindUObject(TargetRader.Get(), &URadar::CallWarning);
        TargetRader->Trace(this);
    }
}

void ASemiActiveAntiAirMissile::Release()
{
    ResetMissileBind();

    SetActorEnableCollision(false);
    //SetActorHiddenInGame(true);
    SetActorTickEnabled(false);
    BodyMesh->SetVisibility(false);
    FCSInfo = nullptr;

    FTimerHandle timerHandle;
    GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &ASemiActiveAntiAirMissile::RetriveObject, 5.0f, false);
}

bool ASemiActiveAntiAirMissile::Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked, AMissile*& output)
{
    if(!Super::Shoot(owner, locked, output))
        return false;

    if (output == nullptr)
        return false;

    ASemiActiveAntiAirMissile* newInstance = static_cast<ASemiActiveAntiAirMissile*>(output);
    Owner = Cast<AAircraft>(owner);
    newInstance->FCSInfo = &Owner->GetFireControlSystem()->GetCurrentTargets();

    return true;
}
