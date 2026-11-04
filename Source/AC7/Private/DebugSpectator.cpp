// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugSpectator.h"

#include "SelectPanel.h"
#include "UIHandler.h"
#include "StageChangeUIUnit.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Aircraft.h"
#include "PlayerPilot.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

//#include "EnhancedInputComponent.h"

ADebugSpectator::ADebugSpectator()
{
	PrimaryActorTick.bCanEverTick = true;
    ThirdCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdCameraArm"));
}

void ADebugSpectator::PostInitProperties()
{
	Super::PostInitProperties();
}

void ADebugSpectator::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ADebugSpectator::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);


    //if (UEnhancedInputComponent* Input =
    //    Cast<UEnhancedInputComponent>(PlayerInputComponent))
    //{
    //    Input->BindAction(
    //        InteractAction,
    //        ETriggerEvent::Started,
    //        this,
    //        &AMyCharacter::Interact
    //    );
    //}
}

void ADebugSpectator::BeginPlay()
{
	Super::BeginPlay();

    if (ThirdCameraArm != nullptr)
        NewObject<UCameraComponent>(ThirdCameraArm, TEXT("BaseCamera"));
    for (TActorIterator<AVehicleBase> Iterator(GetWorld()); Iterator; ++Iterator)
        traceableTargetList.Add(*Iterator);
    AttachedActor.Reset();
}

void ADebugSpectator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


    APlayerController* controller = Cast<APlayerController>(GetController());

    double left = static_cast<double>(controller->IsInputKeyDown(EKeys::Left) || controller->IsInputKeyDown(EKeys::D));
    double right = static_cast<double>(controller->IsInputKeyDown(EKeys::Right) || controller->IsInputKeyDown(EKeys::A));
    double up = static_cast<double>(controller->IsInputKeyDown(EKeys::W) || controller->IsInputKeyDown(EKeys::Up));
    double down = static_cast<double>(controller->IsInputKeyDown(EKeys::S) || controller->IsInputKeyDown(EKeys::Down));
    double space = static_cast<double>(controller->IsInputKeyDown(EKeys::SpaceBar)/* || controller->IsInputKeyDown(EKeys::Down)*/);
    double c = static_cast<double>(controller->IsInputKeyDown(EKeys::C)/* || controller->IsInputKeyDown(EKeys::Down)*/);

    double xValue = Input1.X = right - left;
    double yValue = Input1.Y = up - down;
    double zValue = Input2.X = Input1.Z = space - c;
    double wValue = Input2.Y = static_cast<double>(controller->WasInputKeyJustPressed(EKeys::Q)) - static_cast<double>(controller->WasInputKeyJustPressed(EKeys::E));

    if (AttachedActor.IsValid())
    {
        ArmRotator.Pitch += xValue * MoveSpeed * DeltaTime;
        ArmRotator.Yaw += yValue * MoveSpeed * DeltaTime;

        SetActorLocation(AttachedActor->GetActorLocation() + ArmRotator.RotateVector(GetActorForwardVector() * -150.0f));
        SetActorRotation(AttachedActor->GetActorRotation());
    }
    //else
        //SetActorLocation(GetActorLocation() + (GetActorForwardVector() * Input1.X + GetActorRightVector() * Input1.Y + GetActorUpVector() * Input1.Z * MoveSpeed * DeltaTime));
    if (wValue < 0)
    {
        Index++;
        while (Index < traceableTargetList.Num())
        {
            if (traceableTargetList[Index].IsValid())
                break;
        
			traceableTargetList.RemoveAt(Index);
        }
        
        if (Index >= traceableTargetList.Num())
        {
            traceableTargetList.Reset();
            for (TActorIterator<AVehicleBase> Iterator(GetWorld()); Iterator; ++Iterator)
                traceableTargetList.Add(*Iterator);
        
            if (traceableTargetList.IsEmpty())
            {
                Index = -1;
                AttachedActor.Reset();
            }
            else
            {
                Index = 0;
                if (traceableTargetList.Num() > 1 && AttachedActor.IsValid() && traceableTargetList[Index] == AttachedActor)
                    Index++;
            }
        }

        if (Index < 0 || Index >= traceableTargetList.Num())
            AttachedActor.Reset();
        else
            AttachedActor = traceableTargetList[Index];
    }
    else if (wValue > 0)
    {
        Index--;
        while (Index >= 0)
        {
            if (traceableTargetList[Index].IsValid())
                break;

            traceableTargetList.RemoveAt(Index--);
        }

        if (Index < 0)
        {
            traceableTargetList.Reset();
            for (TActorIterator<AVehicleBase> Iterator(GetWorld()); Iterator; ++Iterator)
                traceableTargetList.Add(*Iterator);

            if (traceableTargetList.IsEmpty())
            {
                Index = -1;
                AttachedActor.Reset();
            }
            else
            {
                Index = traceableTargetList.Num() - 1;
                if (traceableTargetList.Num() > 1 && AttachedActor.IsValid() && traceableTargetList[Index] == AttachedActor)
                    Index--;
            }
        }

        if (Index < 0 || Index >= traceableTargetList.Num())
			AttachedActor.Reset();
        else
            AttachedActor = traceableTargetList[Index];
    }
}
