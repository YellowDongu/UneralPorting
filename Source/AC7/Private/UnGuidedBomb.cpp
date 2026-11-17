// Fill out your copyright notice in the Description page of Project Settings.

#include "UnGuidedBomb.h"

#include "Components/WidgetComponent.h"

#include "ObjectFactory.h"
#include "Aircraft.h"
#include "DrawDebugHelpers.h"
#include "FireControlSystem.h"

AUnGuidedBomb::AUnGuidedBomb()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    MissileID = EWeaponID::UGB;
    FlyDistance = ConvertFromDistance(100000.0);
}

void AUnGuidedBomb::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    //static const FVector Gravity = FVector(0.0f, 0.0f, -980.0f); // Unreal Engine의 기본 중력 가속도라는 거 같다
    //static const FVector Gravity = FVector(0.0f, 0.0f, -980.0f * 0.75f);

    ElapsedTime += DeltaTime;

    if (ElapsedTime >= LandingTime)
    {
        Explode();
        if (BombIndicatorHUD.IsValid())
            BombIndicatorHUD->Destroy();
        BombIndicatorHUD.Reset();
        return;
    }
    
    FVector NewPosition = CalculatePosition(StartPosition, InitialVelocity, ElapsedTime);
    FVector CurrentVelocity = InitialVelocity + BombGravity * ElapsedTime;

    SetActorLocationAndRotation(NewPosition, CurrentVelocity.ToOrientationRotator());

    const FVector CurrentLocation = GetActorLocation();
    //DrawDebugLine(GetWorld(), PreviousLocation, CurrentLocation, FColor::Red, false, LandingTime, 0, 200.0f);
    PreviousLocation = CurrentLocation;
}

void AUnGuidedBomb::Initialize(AMissile* origin, TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked)
{
    AAircraft* aircraft = static_cast<AAircraft*>(owner.Get());
    const FControl& control = aircraft->GetControl();
    FVector location = aircraft->GetActorLocation();

    LandingTime = aircraft->GetFireControlSystem()->GetCalculatedBombTime();
    if (LandingTime <= 0)
    {
        Release();
        if (BombIndicatorHUD.IsValid())
            BombIndicatorHUD->Destroy();
        BombIndicatorHUD.Reset();
        return;
    }

    SetActorEnableCollision(true);
    SetActorHiddenInGame(false);
    SetActorTickEnabled(true);
    PrimaryActorTick.bStartWithTickEnabled = bIsProjectile = true;

    SetActorLocationAndRotation(location, GetActorRotation(), true);
    ElapsedTime = 0.0f;
    bLanded = false;
    StartPosition = PreviousLocation = location;
    InitialVelocity = aircraft->GetActorForwardVector() * ConvertFromVelocity(static_cast<double>(control.Velocity));

}

int AUnGuidedBomb::Shoot(TWeakObjectPtr<class AVehicleBase> owner, FTargetInfo* locked)
{
    if (owner == nullptr || bIsCoolTime)
        return -1;

    //newInstance->GiveRadarWarn.Unbind();
    //newInstance->mesh->SetVisibility(true);
    //newInstance->Trail = newInstance->FindComponentByClass<UNiagaraComponent>();

    //mesh->SetVisibility(true);

    TWeakObjectPtr<AUnGuidedBomb> newInstance = Cast<AUnGuidedBomb>(CreateProjectile());
    if (!newInstance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[AUnGuidedBomb::Shoot] CREATE ERROR"));
        return -1;
    }
    newInstance->Initialize(this, owner, nullptr);
    if (BombIndicatorHUD.IsValid())
    {
        newInstance->BombIndicatorHUD = GetWorld()->SpawnActor<AActor>(BombIndicatorHUD->GetClass(), BombIndicatorHUD->GetActorTransform(), FActorSpawnParameters());
        UWidgetComponent* widget = static_cast<UWidgetComponent*>(newInstance->BombIndicatorHUD->GetComponentByClass(UWidgetComponent::StaticClass()));
        widget->UpdateWidget();
        UMaterialInstanceDynamic* material = widget->GetMaterialInstance();
        if(IsValid(material))
            material->SetVectorParameterValue(TEXT("Color"), RedHUD);
        /*
        else
        {
            widget->UpdateMaterialInstanceParameters()
            widget->UpdateMaterialInstance()
            widget->OnMaterialInstanceUpdated.AddLambda([this]()
                                                                 {
                                                                     UMaterialInstanceDynamic* MID =
                                                                         WidgetComponent->GetMaterialInstance();

                                                                     if (MID)
                                                                     {
                                                                         MID->SetScalarParameterValue(
                                                                             TEXT("MyParameter"), 1.0f);
                                                                     }
                                                                 });
        }
        */
    }
        
        
    //newInstance->BombIndicatorHUD->SetActorLocation(TrackedPosition);

    bIsCoolTime = true;
    GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &AUnGuidedBomb::CoolTimeEnd, coolTime, false);

    return 1;
}

void AUnGuidedBomb::Explode()
{
	UWorld* world = GetWorld();
    FVector location = GetActorLocation();
    world->GetSubsystem<UObjectFactory>()->Explosion(world, location, 550.0f, 2.0f);

    //FCollisionQueryParams Params;
    FCollisionObjectQueryParams ObjectParams;

    //ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
    //ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel1); // radar
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Vehicle);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
    ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Destructible);

    TArray<FOverlapResult> list;
    world->OverlapMultiByObjectType(list, location, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(BlastArea));

    DrawDebugSphere(world, location, BlastArea, 8, FColor::Red, false, 10.0f);
    DrawDebugSphere(world, location, BlastArea * 0.6f, 8, FColor::Red, false, 10.0f);
    DrawDebugSphere(world, location, BlastArea * 0.2f, 8, FColor::Red, false, 10.0f);

    TSet<AVehicleBase*> hitted;

    for (auto& item : list)
    {
        //AActor* Actor = item.GetActor();
        //
        //if (Comp && Actor)
        //{
        //    UE_LOG(LogTemp, Warning,
        //           TEXT("Actor=%s Component=%s Location=%s BoundsOrigin=%s BoundsExtent=%s ObjectType=%d"),
        //           *Actor->GetName(),
        //           *Comp->GetName(),
        //           *Comp->GetComponentLocation().ToString(),
        //           *Comp->Bounds.Origin.ToString(),
        //           *Comp->Bounds.BoxExtent.ToString(),
        //UPrimitiveComponent* Comp = item.GetComponent();
        //           (int32)Comp->GetCollisionObjectType());
        //}

        AVehicleBase* vehicle = Cast<AVehicleBase>(item.GetActor());

        if (vehicle == nullptr || hitted.Contains(vehicle))
            continue;

        hitted.Add(vehicle);
        vehicle->HPDamage(vehicle->GetMask().IsLand() ? damage : damage / 2);
        UE_LOG(LogTemp, Log, TEXT("[AUnGuidedBomb::Tick] Damage -> %s"), *vehicle->GetName());
    }
    Release();
}
