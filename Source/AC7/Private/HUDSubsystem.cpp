// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDSubsystem.h"
#include "FrontHUD.h"
#include "PlayerPilot.h"
#include "Aircraft.h"

void UHUDSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    // 생성자에서만 가능한 것
	//ConstructorHelpers::FClassFinder<UFrontHUD> FrontHUDClass(TEXT("/Blueprint/FrontHUD"));
    //if (FrontHUDClass.Succeeded()) FrontHUDBlueprint = FrontHUDClass.Class;

    FrontHUDBlueprint = LoadClass<UFrontHUD>(nullptr, TEXT("/Game/Blueprint/FrontHUD.FrontHUD_C"));
    

}

void UHUDSubsystem::Deinitialize()
{
    FrontHUD = nullptr;
}

void UHUDSubsystem::CreateHUD()
{
    if (FrontHUD != nullptr)
        return;

    FrontHUD = CreateWidget<UFrontHUD>(GetWorld(), FrontHUDBlueprint);

    if (FrontHUD != nullptr)
        FrontHUD->AddToViewport();
}

void UHUDSubsystem::LinkHUD(AAircraft* aircraft, APlayerPilot* pilot)
{
    if (FrontHUD == nullptr)
    {
        CreateHUD();
        if (FrontHUD == nullptr)
            return;
    }

    //pilot->AttachUI(FrontHUD);
    FrontHUD->BindVehicle(aircraft, pilot);
}
