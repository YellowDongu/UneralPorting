// Fill out your copyright notice in the Description page of Project Settings.


#include "Missile.h"
#include "VehicleBase.h"
#include "Radar.h"
#include "ObjectFactory.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

AMissile::AMissile()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AMissile::CoolTimeEnd()
{
    bIsCoolTime = false;
    BodyMesh->SetVisibility(true);
}

bool AMissile::Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked) { AMissile* dummy; return Shoot(owner, locked, dummy); }

void AMissile::Initialize(AMissile* origin, TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<AVehicleBase> locked)
{
    GiveRadarWarn.Unbind();
    SetActorEnableCollision(true);
    SetActorHiddenInGame(false);
    SetActorTickEnabled(true);
    BodyMesh->SetVisibility(true);

    //Trail->Deactivate();
    SetActorLocationAndRotation(origin->GetActorLocation(), origin->GetActorRotation(), true);
    bIsProjectile = true;
    FlyDistance = MaxRange * 1.5f;
    //Trail->ReinitializeSystem();
    if (IsValid(Trail))
        Trail->Activate();

    Target = locked;
    if (!IsAlive(Target))
    {
        UE_LOG(LogTemp, Error, TEXT("[AMissile::Shoot] !IsAlive(Target)"));
        return;
    }
    TargetRader = locked->FindComponentByClass<URadar>();
    if (TargetRader == nullptr)
        return;

    GiveRadarWarn.BindUObject(TargetRader.Get(), &URadar::CallWarning);
    TargetRader->Trace(this);
}

bool AMissile::Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked, AMissile*& output)
{
    if (bIsCoolTime)
        return false;

    TWeakObjectPtr<AMissile> newInstance = CreateProjectile();
    if (newInstance == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[AMissile::Shoot] CREATE ERROR"));
        return false;
    }

    newInstance->Initialize(this, owner, locked);
    output = newInstance.Get();

    bIsCoolTime = true;
    GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &AMissile::CoolTimeEnd, coolTime, false);
    BodyMesh->SetVisibility(false);

    return true;
}

int AMissile::Shoot(TWeakObjectPtr<class AVehicleBase> owner, FTargetInfo* locked)
{
    if (bIsCoolTime || locked == nullptr)
        return -1;

    if (locked->Lock >= 0.001f)
        return 0;

    Shoot(owner, locked->Target);
    return 1;
}

void AMissile::ForceChangeTarget(TWeakObjectPtr<class AVehicleBase> nextTarget)
{ 
    GiveRadarWarn.Unbind();
    if (TargetRader != nullptr)
        TargetRader->UnTrace(this);

    Target = nextTarget;
    TWeakObjectPtr<AVehicleBase> casted = Cast<AVehicleBase>(nextTarget);
    if (casted == nullptr) return;

    TargetRader = casted->FindComponentByClass<URadar>();
    if (TargetRader == nullptr) return;
    GiveRadarWarn.BindUObject(TargetRader.Get(), &URadar::CallWarning);
    TargetRader->Trace(this);
}

void AMissile::Lock(FTargetInfo& targetData, float DeltaTime, const FVector& forward, const FVector& position)
{
    FVector direction = targetData.Target->GetActorLocation() - position;
    targetData.CalculateDistance(direction, false, true);

    bool lockable = targetData.DistanceSquared < DistanceSquare && FVector::DotProduct(direction.GetSafeNormal(), forward) > AngleCos;
    targetData.Lock = FMath::Clamp(targetData.Lock + (lockable ? -DeltaTime : DeltaTime), 0.0f, 1.0f);
}

void AMissile::Lock(FTargetInfo& targetData, float DeltaTime, const TWeakObjectPtr<class AVehicleBase>& owner)
{
    FVector direction = targetData.Target->GetActorLocation() - owner->GetActorLocation();
    FVector forward = owner->GetActorForwardVector();
    targetData.CalculateDistance(direction, false, true);

    bool lockable = targetData.DistanceSquared < DistanceSquare && FVector::DotProduct(direction.GetSafeNormal(), forward) > AngleCos;
    targetData.Lock = FMath::Clamp(targetData.Lock + (lockable ? -DeltaTime : DeltaTime), 0.0f, 1.0f);
}



TWeakObjectPtr<AMissile> AMissile::CreateProjectile(TWeakObjectPtr<class AVehicleBase> targets)
{
    UObjectFactory* objectFactory = GetWorld()->GetSubsystem<UObjectFactory>();
    if (objectFactory == nullptr) return nullptr;

    return objectFactory->Instantiate(this);
}

TWeakObjectPtr<AMissile> AMissile::CreateProjectile(void)
{
    UObjectFactory* objectFactory = GetWorld()->GetSubsystem<UObjectFactory>();
    if (objectFactory == nullptr) return nullptr;

    return objectFactory->Instantiate(this);
}

void AMissile::Release(void)
{
    ResetMissileBind();

    SetActorEnableCollision(false);
    SetActorHiddenInGame(true);
    SetActorTickEnabled(false);
    RetriveObject();
}

void AMissile::RetriveObject()
{
    if (GetWorld() == nullptr)
    {
        Destroy();
        return;
    }

    UObjectFactory* objectFactory = GetWorld()->GetSubsystem<UObjectFactory>();
    if (objectFactory != nullptr)
    {
        if (IsValid(Trail))
            Trail->Deactivate();
        SetActorEnableCollision(false);
        SetActorHiddenInGame(true);
        SetActorTickEnabled(false);
        objectFactory->Release(this);
    }
    else
        Destroy();
}

void AMissile::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    BodyMesh = FindComponentByClass<UStaticMeshComponent>();
    Trail = FindComponentByClass<UNiagaraComponent>();
    MaxRange = ConvertFromDistance(MaxRange);
    AimLayer.Initialize();
}

void AMissile::BeginPlay(void)
{
    Super::BeginPlay();

    ImpactDistance = ImpactDistance * ImpactDistance;
    DistanceSquare = MaxRange * MaxRange;
    AngleCos = FMath::Cos(FMath::DegreesToRadians(lockAngle));
}

void AMissile::ResetMissileBind()
{
    Target.Reset();
    GiveRadarWarn.Unbind();
    if (TargetRader != nullptr)
        TargetRader->UnTrace(this);
    TargetRader.Reset();
    BodyMesh->SetVisibility(true);
}
