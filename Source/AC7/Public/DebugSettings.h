// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlightMovement.h"
#include "Engine/DeveloperSettings.h"
#include "DebugSettings.generated.h"

/**
 * 
 */

UCLASS(Config = Game, DefaultConfig)
class AC7_API UDebugSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, Config, Category = "Gameplay")
    bool bDebugSpectatorMode = false;
    UPROPERTY(EditAnywhere, Config, Category = "SoundDebug")
    float shortDuration = 3;
    UPROPERTY(EditAnywhere, Config, Category = "SoundDebug")
    float longDuration = 1.5;
};