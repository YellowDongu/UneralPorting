// Fill out your copyright notice in the Description page of Project Settings.

#include "MissionMode.h"
#include "RadarUI.h"
#include "EngineUtils.h"
#include "UIHandler.h"
#include "StageChangeUIUnit.h"
#include "PlayerPilot.h"
#include "DebugSettings.h"

AMissionMode::AMissionMode()
{
}

void AMissionMode::BeginPlay()
{
    Super::BeginPlay();

    FVector leftBottom{ FVector::ZeroVector }, rightTop{ FVector::OneVector };
    
    ULevel* Level = GetWorld()->GetCurrentLevel();
    
    for (AActor* actor : Level->Actors)
    {
        if (!IsValid(actor))
            continue;

        if (actor->GetActorNameOrLabel() == TEXT("LBBoarder"))
            leftBottom = actor->GetActorLocation();
        else if (actor->GetActorNameOrLabel() == TEXT("RTBoarder"))
            rightTop = actor->GetActorLocation();
        //else if (actor->ActorHasTag(TEXT("LBBoarder")))
        //    leftBottom = actor->GetActorLocation();
        //else if (actor->ActorHasTag(TEXT("RTBoarder")))
        //    rightTop = actor->GetActorLocation();
    }
    
    UE_LOG(LogTemp, Error, TEXT("[AMissionMode::BeginPlay] LBBoarder : %s, RTBoarder : "), *leftBottom.ToString(), *rightTop.ToString());
    URadarUI::SetBoarder(leftBottom, rightTop);
}

void AMissionMode::PostInitProperties()
{
    Super::PostInitProperties();

    if (GetSpectatorMode())
    {
        PlayerControllerClass = APlayerController::StaticClass();
        bStartPlayersAsSpectators = true;
    }
    //if (PlayerController != nullptr)
    //    PlayerControllerClass = PlayerController;
    //else
    //    UE_LOG(LogTemp, Error, TEXT("[AMissionMode::AMissionMode] PlayerController == nullptr"));
    //if (PlayerControllerClass != nullptr)
    //    UE_LOG(LogTemp, Error, TEXT("[AMissionMode::AMissionMode] Controller : %s"), *PlayerControllerClass->GetDefaultObject()->GetClass()->GetName());
//    if (PlayerControllerClass == nullptr)
//        PlayerControllerClass = PlayerController;
}

void AMissionMode::PreInitializeComponents()
{
    Super::PreInitializeComponents();

}

void AMissionMode::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

void AMissionMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // 멀티 시 분리해서 RPC로 만들어야될 수도 있음
    TWeakObjectPtr<AUIHandler> hud = NewPlayer->GetHUD<AUIHandler>();

    if (hud.IsValid())
        return;

    
    hud->GetStageChangeUIUnit()->FadeOut(true);
    FTimerHandle dummy;
    GetWorld()->GetTimerManager().SetTimer(dummy, [hud]()-> void { if (hud.IsValid()) return; hud->GetStageChangeUIUnit()->FadeOut(false, 0.5f); }, 0.5f, false);
}

void AMissionMode::SetBoarder(const FVector& leftBottom, const FVector& rightTop) { URadarUI::SetBoarder(leftBottom, rightTop); }

void AMissionMode::TriggerWinLocal()
{
    for (FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator)
    {
        if (Cast<APlayerPilot>(iterator->Get()) == nullptr)
            continue;

        TriggerWinMission((*iterator)->GetHUD<AUIHandler>());
    }
}

void AMissionMode::TriggerWinMission(AUIHandler* hud)
{
    if (hud == nullptr)
        return;

    UE_LOG(LogTemp, Error, TEXT("[AMissionMode::TriggerWinLocal] Active"));
    //hud->GetStageChangeUIUnit()->FadeOut(true, 1.0f);
    hud->GetStageChangeUIUnit()->ReturnMainMenu(true);
}

bool AMissionMode::GetSpectatorMode()
{
    const UDebugSettings* debugSetting = GetDefault<UDebugSettings>();
    if (debugSetting == nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("DEBUG / debugSetting == nullptr"));
        return false;
    }

    return debugSetting->bDebugSpectatorMode;
}

/*

APlayerController* AMissionMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
    return Super::SpawnPlayerController(InRemoteRole, Options);
    APlayerController* controller;

    PRAGMA_DISABLE_DEPRECATION_WARNINGS
        if (Options.Contains(FString(TEXT("SpectatorOnly=1"))) && ReplaySpectatorPlayerControllerClass != nullptr)
        {
            controller = Super::SpawnReplayPlayerController(InRemoteRole, FVector::ZeroVector, FRotator::ZeroRotator);
        }

    controller = Super::SpawnPlayerController(InRemoteRole, FVector::ZeroVector, FRotator::ZeroRotator);
    PRAGMA_ENABLE_DEPRECATION_WARNINGS

    UE_LOG(LogTemp, Warning, TEXT("SpawnPlayerController Class: %s"),
           *GetNameSafe(PlayerControllerClass));
    UE_LOG(LogTemp, Warning, TEXT("SpawnedPlayerController Class: %s"),
           *GetNameSafe(controller));

    UE_LOG(LogTemp, Warning, TEXT("Castable to PlayerControllerClass Class: %s"),
           Cast<APlayerPilot>(controller) == nullptr ? TEXT("true") : TEXT("false"));

    FActorSpawnParameters Params;
    APlayerController* Result =
        GetWorld()->SpawnActor<APlayerController>(
            PlayerControllerClass,
            Params);

    UE_LOG(LogTemp, Warning,
           TEXT("Manual Spawn Result: %s"),
           *GetNameSafe(Result));

    return controller;
}

APlayerController* AMissionMode::SpawnPlayerControllerCommon(ENetRole InRemoteRole, FVector const& SpawnLocation, FRotator const& SpawnRotation, TSubclassOf<APlayerController> InPlayerControllerClass)
{
    return Super::SpawnPlayerControllerCommon(InRemoteRole, SpawnLocation, SpawnRotation, InPlayerControllerClass);
    UE_LOG(LogTemp, Warning, TEXT("SpawnPlayerControllerCommon // SpawnPlayerController Class: %s"),
           *GetNameSafe(PlayerControllerClass));
    UE_LOG(LogTemp, Warning, TEXT("SpawnPlayerControllerCommon // SpawnedPlayerController Class: %s"),
           *GetNameSafe(InPlayerControllerClass));

    return Super::SpawnPlayerControllerCommon(InRemoteRole, SpawnLocation, SpawnRotation, PlayerControllerClass);
}

*/