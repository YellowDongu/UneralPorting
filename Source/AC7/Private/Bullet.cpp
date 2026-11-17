// Fill out your copyright notice in the Description page of Project Settings.
#include "Bullet.h"
#include "ObjectFactory.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "VehicleBase.h"

ABullet::ABullet()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bTickEvenWhenPaused = false;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

}

void ABullet::Initialize()
{
    Body = Cast<UStaticMeshComponent>(GetRootComponent());
    Body->SetNotifyRigidBodyCollision(true);
    Body->OnComponentHit.AddDynamic(this, &ABullet::OnTriggerHit);
    ShootSpeed = 330.0;
    CurrentLifeTime = LifeTime;
    //Trail = Cast<UNiagaraComponent>(Body->GetAttachChildren()[0]);
    //
    //FNiagaraVariable DirectionParam(FNiagaraTypeDefinition::GetVec3Def(), TEXT("User.MoveVector"));
    //FlareBodyDirection.Init(Body->GetOverrideParameters(), DirectionParam);
    //UE_LOG(LogTemp, Warning, TEXT("Sim=%d Collision=%d Physics=%d Query=%d HasPhysics=%d"), Body->IsSimulatingPhysics(), (int32)Body->GetCollisionEnabled(), Body->IsPhysicsCollisionEnabled(), Body->IsQueryCollisionEnabled(), Body->IsPhysicsStateCreated());

}

void ABullet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CurrentLifeTime -= DeltaTime;

    //SetActorLocation(GetActorLocation() + Direction * CurrentSpeed * DeltaTime);
    if (GetActorLocation().Z < 0.0 || CurrentLifeTime <= 0.0f)
    {
        Release();
        return;
    }

    //CurrentSpeed = FMath::Lerp(ShootSpeed, MinimumSpeed, 1.0f - CurrentLifeTime / LifeTime);
    //FVector Direction = GetActorForwardVector();
    //SetActorLocation(GetActorLocation() + Direction * CurrentSpeed * DeltaTime);
}

void ABullet::OnTriggerHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    AVehicleBase* vehicle = Cast<AVehicleBase>(OtherActor);
    if (vehicle != nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[ABullet::OnTriggerHit] %s -> %s"), *GetName(), *OtherActor->GetName());

        vehicle->HPDamage(Damage);

    }
    else
    {
        /*
        UE_LOG(LogTemp, Error, TEXT("[ABullet::OnTriggerHit] :::: %s -> %s"), *GetName(), *OtherActor->GetName());

        //DrawDebugSphere(GetWorld(), GetActorLocation(), 10.0f, 8, FColor::Red, false, 2.0f);
        UBoxComponent* BoxComp = Cast<UBoxComponent>(HitComponent);
        USphereComponent* Sphere = Cast<USphereComponent>(HitComponent);
        UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(HitComponent);

        if (BoxComp)
            DrawDebugBox(GetWorld(), BoxComp->GetComponentLocation(), BoxComp->GetScaledBoxExtent(), BoxComp->GetComponentQuat(), FColor::Red, false, 2.0f, 0, 2.0f);
        else if (Sphere)
            DrawDebugSphere(GetWorld(), Sphere->GetComponentLocation(), Sphere->GetScaledSphereRadius(), 16, FColor::Red, false, 2.0f);
        else if (Capsule)
            DrawDebugCapsule(GetWorld(), Capsule->GetComponentLocation(), Capsule->GetScaledCapsuleHalfHeight(), Capsule->GetScaledCapsuleRadius(), Capsule->GetComponentQuat(), FColor::Red, false, 2.0f, 0, 2.0f);
        else
        {
            UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(HitComponent);
            if (!MeshComp) return;

            UStaticMesh* StaticMesh = MeshComp->GetStaticMesh();
            if (!StaticMesh) return;

            UBodySetup* BodySetup = StaticMesh->GetBodySetup();
            if (!BodySetup) return;

            const FKAggregateGeom& AggGeom = BodySetup->AggGeom;
            const FTransform ComponentTransform = MeshComp->GetComponentTransform();

            const float Duration = 5.0f;
            const float Thickness = 2.0f;
            for (const FKBoxElem& Box : AggGeom.BoxElems)
            {
                FTransform BoxTransform = Box.GetTransform();
                FTransform WorldTransform = BoxTransform * ComponentTransform;
                DrawDebugBox(GetWorld(), WorldTransform.GetLocation(), FVector(Box.X, Box.Y, Box.Z), WorldTransform.GetRotation(), FColor::Red, false, Duration, 0, Thickness);
            }
        }
        */
    }


    Release();

}

void ABullet::Release()
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
        Body->SetSimulatePhysics(false);
        objectFactory->Release(this);
    }
    else
        Destroy();
}

void ABullet::Shoot(const FVector& position, const FRotator& rotation, float velocity, int damage)
{
    Damage = damage;
    CurrentLifeTime = LifeTime;

    Body->SetSimulatePhysics(true);
    SetActorEnableCollision(true);
    SetActorHiddenInGame(false);
    SetActorTickEnabled(true);

    SetActorLocationAndRotation(position, rotation);
    Body->AddImpulse(GetActorForwardVector() * velocity * 50.0f);
    //UE_LOG(LogTemp, Error, TEXT("[ABullet::Shoot] %3f : %3f : %3f / %3f : %3f : %3f"), position.X, position.Y, position.Z, Body->GetPhysicsLinearVelocity().X, Body->GetPhysicsLinearVelocity().Y, Body->GetPhysicsLinearVelocity().Z);
}