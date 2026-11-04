// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpectatorPawn.h"
#include "Defined.h"

#include "DebugSpectator.generated.h"

/**
 *
 */
UCLASS()
class AC7_API ADebugSpectator : public ASpectatorPawn
{
    GENERATED_BODY()
public:
    ADebugSpectator();
    virtual ~ADebugSpectator() = default;

    virtual void BeginPlay() override;
    virtual void PostInitProperties() override;
    //virtual void PreInitializeComponents() override;
    virtual void PostInitializeComponents() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
private:
    bool bTickActive{ false };
    float MoveSpeed{ 1000.0f };
    TWeakObjectPtr<AActor> AttachedActor{ nullptr };
    //FVector Offset{ FVector::ZeroVector };
    
    FRotator ArmRotator;

    int Index = -1;
    TSet<TWeakObjectPtr<class AVehicleBase>> traceableTargets;
    TArray<TWeakObjectPtr<class AVehicleBase>> traceableTargetList;

    UPROPERTY()
    class USceneComponent* FirstCameraArm{ nullptr };
    UPROPERTY()
    class USpringArmComponent* ThirdCameraArm{ nullptr };

    FVector Input1;
    FVector Input2;
};
