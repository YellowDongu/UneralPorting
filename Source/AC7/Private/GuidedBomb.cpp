// Fill out your copyright notice in the Description page of Project Settings.
#include "GuidedBomb.h"
#include "GroundVehicle.h"
#include "ObjectFactory.h"

AGuidedBomb::AGuidedBomb()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    MissileID = EWeaponID::LGB;
}

void AGuidedBomb::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGuidedBomb::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ElapsedTime += DeltaTime;

    if (IsValid(Target.Get()) && !Target->IsHidden())
        TrackedPosition = Target->GetActorLocation();

    if (ElapsedTime >= LandingTime)
    {
        UE_LOG(LogTemp, Log, TEXT("[AStandardMissile::Tick] Collide -> %s"), *Target->GetName());
        GetWorld()->GetSubsystem<UObjectFactory>()->Explosion(GetWorld(), TrackedPosition, 150.0f, 2.0f);
        Explode();
    }

    FVector nextPosition = FMath::Lerp(StartPosition, TrackedPosition, ElapsedTime / LandingTime);
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), (nextPosition - GetActorLocation()).Rotation(), DeltaTime, rotationSpeed));
    SetActorLocation(nextPosition);
}

bool AGuidedBomb::Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked, AMissile*& output)
{
    if (bIsCoolTime || !locked.IsValid())
        return false;

    TWeakObjectPtr<AGuidedBomb> newInstance = static_cast<AGuidedBomb*>(CreateProjectile().Get());

    newInstance->SetActorEnableCollision(true);
    newInstance->SetActorHiddenInGame(false);
    newInstance->SetActorTickEnabled(true);
    newInstance->PrimaryActorTick.bStartWithTickEnabled = true;

    newInstance->Target = locked;

    newInstance->SetActorLocationAndRotation(GetActorLocation(), GetActorRotation(), true);
    newInstance->bIsProjectile = true;

    bIsCoolTime = true;
    GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &AGuidedBomb::CoolTimeEnd, coolTime, false);

    newInstance->StartPosition = GetActorLocation();
    newInstance->TrackedPosition = locked->GetActorLocation();
    newInstance->LandingTime = static_cast<float>((newInstance->TrackedPosition.Z - GetActorLocation().Z) / 980.0) * 0.8f;
    
    output = this;
    return true;
}

void AGuidedBomb::Explode()
{
    UWorld* world = GetWorld();
    FCollisionObjectQueryParams ObjectParams;

    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Vehicle);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Destructible);

    TArray<FOverlapResult> list;
    world->OverlapMultiByObjectType(list, TrackedPosition, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(BlastArea));

    DrawDebugSphere(world, TrackedPosition, BlastArea, 8, FColor::Red, false, 10.0f);
    DrawDebugSphere(world, TrackedPosition, BlastArea * 0.6f, 8, FColor::Red, false, 10.0f);
    DrawDebugSphere(world, TrackedPosition, BlastArea * 0.2f, 8, FColor::Red, false, 10.0f);

    TSet<AVehicleBase*> hitted;

    for (auto& item : list)
    {
        AVehicleBase* vehicle = Cast<AVehicleBase>(item.GetActor());

        if (vehicle == nullptr || hitted.Contains(vehicle))
            continue;

        hitted.Add(vehicle);
        vehicle->HPDamage(damage);
        UE_LOG(LogTemp, Log, TEXT("[AUnGuidedBomb::Tick] Collide -> %s"), *vehicle->GetName());
    }

    world->GetSubsystem<UObjectFactory>()->Explosion(world, TrackedPosition, 550.0f, 2.0f);
    Release();
}
