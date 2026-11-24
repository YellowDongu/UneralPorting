// Fill out your copyright notice in the Description page of Project Settings.
#include "GuidedBomb.h"

#include "Components/WidgetComponent.h"

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
    ClosedArea = MaxRange * 0.21;
    ClosedArea = ClosedArea * ClosedArea;
}

void AGuidedBomb::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (ElapsedTime >= LandingTime)
    {
        UE_LOG(LogTemp, Log, TEXT("[AStandardMissile::Tick] Impact"));
        GetWorld()->GetSubsystem<UObjectFactory>()->Explosion(GetWorld(), TrackedPosition, 150.0f, 2.0f);
        Explode();
        if (BombIndicatorHUD.IsValid())
            BombIndicatorHUD->Destroy();
        BombIndicatorHUD.Reset();
        return;
    }

    ElapsedTime += DeltaTime;

    if (IsAlive(Target))
    {
        TrackedPosition = Target->GetActorLocation();
        if (BombIndicatorHUD.IsValid())
            BombIndicatorHUD->SetActorLocation(TrackedPosition);
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
    if (!newInstance.IsValid()) return false;

    newInstance->SetActorLocationAndRotation(GetActorLocation(), GetActorRotation(), true);
    newInstance->SetActorEnableCollision(true);
    newInstance->SetActorHiddenInGame(false);
    newInstance->SetActorTickEnabled(true);

    newInstance->Target = locked;
    newInstance->bIsProjectile = true;

    newInstance->StartPosition = GetActorLocation();
    newInstance->TrackedPosition = locked->GetActorLocation();
    newInstance->LandingTime = FMath::Abs(static_cast<float>((newInstance->StartPosition.Z - newInstance->TrackedPosition.Z) / 980.0 * 0.8));
    if (BombIndicatorHUD.IsValid())
    {
        newInstance->BombIndicatorHUD = GetWorld()->SpawnActor<AActor>(BombIndicatorHUD->GetClass(), BombIndicatorHUD->GetActorTransform(), FActorSpawnParameters());
        UWidgetComponent* widget = static_cast<UWidgetComponent*>(newInstance->BombIndicatorHUD->GetComponentByClass(UWidgetComponent::StaticClass()));
        widget->GetMaterialInstance()->SetVectorParameterValue(TEXT("Color"), RedHUD);
    }

    bIsCoolTime = true;
    GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &AGuidedBomb::CoolTimeEnd, coolTime, false);
    output = this;
    return true;
}

void AGuidedBomb::Lock(FTargetInfo& targetData, float DeltaTime, const FVector& forward, const FVector& position)
{
    FVector direction = targetData.Target->GetActorLocation() - position;
    FVector normal = forward;

    direction.Z = normal.Z = 0.0;
    normal = normal.GetSafeNormal();

    targetData.CalculateDistance(direction, false, true);

    bool lockable = targetData.DistanceSquared < ClosedArea || (targetData.DistanceSquared < DistanceSquare && FVector::DotProduct(direction.GetSafeNormal(), normal) > AngleCos);
    targetData.Lock = FMath::Clamp(targetData.Lock + (lockable ? -DeltaTime : DeltaTime), 0.0f, 1.0f);
}

void AGuidedBomb::Lock(FTargetInfo& targetData, float DeltaTime, const TWeakObjectPtr<class AVehicleBase>& owner)
{
    //Lock(targetData, DeltaTime, owner->GetActorForwardVector(), owner->GetActorLocation());
    FVector direction = targetData.Target->GetActorLocation() - owner->GetActorLocation();
    FVector forward = owner->GetActorForwardVector();
    direction.Z = forward.Z = 0.0;
    forward = forward.GetSafeNormal();

    targetData.CalculateDistance(direction, false, true);

    bool lockable = targetData.DistanceSquared < ClosedArea || (targetData.DistanceSquared < DistanceSquare && FVector::DotProduct(direction.GetSafeNormal(), forward) > AngleCos);
    lockable |= targetData.DistanceSquared < ClosedArea;
    targetData.Lock = FMath::Clamp(targetData.Lock + (lockable ? -DeltaTime : DeltaTime), 0.0f, 1.0f);
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

#ifdef DebugDrawActived
    DrawDebugSphere(world, TrackedPosition, BlastArea, 8, FColor::Red, false, 10.0f);
    DrawDebugSphere(world, TrackedPosition, BlastArea * 0.6f, 8, FColor::Red, false, 10.0f);
    DrawDebugSphere(world, TrackedPosition, BlastArea * 0.2f, 8, FColor::Red, false, 10.0f);
#endif
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
