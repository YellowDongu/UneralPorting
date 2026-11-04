// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPawn.h"


ACameraPawn::ACameraPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    // 1. 루트 컴포넌트 설정
    RootComponent = CreateDefaultSubobject<USceneComponent>(L"Root");

    // Camera Arm Setting
    cameraArm = CreateDefaultSubobject<USpringArmComponent>(L"CameraBoom");
    cameraArm->SetupAttachment(RootComponent);
    cameraArm->TargetArmLength = armLength;
    cameraArm->SetRelativeRotation(FRotator(armRotation, 0.f, 0.f));
    cameraArm->bDoCollisionTest = false;       // 장애물에 가려도 카메라 안 당겨지게

    // Setting Camera
    camera = CreateDefaultSubobject<UCameraComponent>(L"TopDownCamera");
    camera->SetupAttachment(cameraArm, USpringArmComponent::SocketName);
}

void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (controller == nullptr)
    {
        controller = Cast<APlayerController>(GetController());
        if (controller == nullptr)
            return;
    }

    // Camera Move
    Movement(DeltaTime);
    // Camera Move
}

void ACameraPawn::Movement(float deltaTime)
{
    FVector Direction = FVector(
        static_cast<double>(controller->IsInputKeyDown(EKeys::W)) - static_cast<double>(controller->IsInputKeyDown(EKeys::S)),
        static_cast<double>(controller->IsInputKeyDown(EKeys::D)) - static_cast<double>(controller->IsInputKeyDown(EKeys::A)), 0.0);

    if (!Direction.IsNearlyZero())
    {
        Direction.Normalize(); //AddActorWorldOffset(Direction * MoveSpeed * deltaTime);
        FVector NewLocation = GetActorLocation() + (Direction * MoveSpeed * deltaTime);
        SetActorLocation(NewLocation);
    }
}
