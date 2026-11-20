#include "GameMaster.h"

#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

#include "PlayerPilot.h"
#include "LevelLoader.h"
#include "BluePrintCollector.h"
#include "UIHandler.h"
#include "DebugSettings.h"

UGameMaster::UGameMaster() : UGameInstance()
{
}

void UGameMaster::Init()
{
    Super::Init();
    GetCollector();
}

const TObjectPtr<UBluePrintCollector>& UGameMaster::GetCollector()
{
    if (Collector == nullptr)
    {
        check(CollectorBluePrint);
        Collector = NewObject<UBluePrintCollector>(this, CollectorBluePrint);
        check(Collector.Get());
    }

    return Collector;
}

void UGameMaster::Shutdown()
{
#if WITH_EDITOR
    AUIHandler::ResetTrigger();
#endif
    Super::Shutdown();
}

void UGameMaster::ChangeLevel(const FName& levelName) { GetSubsystem<ULevelLoader>()->ChangeLevel(levelName); }
void UGameMaster::ReserveLevel(const FString& nextLevel)
{
    ReservedLevelName.Reset();
    auto level = GetCollector()->GetMissionLevelData(nextLevel);
    if (level == nullptr) return;
    ReservedLevel = level->LevelPtr;
    if (!ReservedLevel.IsNull())
        ReservedLevelName = level->LevelLabel;
}
USoundBase* UGameMaster::GetSoundBluePrint(const FString& bgmName) { return GetCollector()->GetSound(bgmName); }

FString UGameMaster::GetMissionName(int index) { return GetCollector()->GetMissionLevelName(index); }
TSoftObjectPtr<UWorld> UGameMaster::FindMissionLevel(int index) { return GetCollector()->GetMissionLevel(index); }
TSoftObjectPtr<UWorld> UGameMaster::FindMissionLevel(const FString& name) { return GetCollector()->GetMissionLevel(name); }

void UGameMaster::OnStart()
{
    Super::OnStart();

    const UDebugSettings* debugSetting = GetDefault<UDebugSettings>();
    if (debugSetting != nullptr && debugSetting->bAircraftDebugMode)
    {
        PlayerSpawnInfomation.VehicleID = debugSetting->Vehicle;
        PlayerSpawnInfomation.SpecialWeapon = debugSetting->SpecialWeapon;
    }
    else
    {
        PlayerSpawnInfomation.VehicleID = EVehicleID::F15E;
        PlayerSpawnInfomation.SpecialWeapon = EWeaponID::END;
    }
}

#include "Camera/CameraComponent.h"
void UGameMaster::DebugCamera()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    if (!PC)
        return;

    AActor* ViewTarget = PC->GetViewTarget();

    if (!ViewTarget)
        return;

    UE_LOG(LogTemp, Warning, TEXT("ViewTarget: %s"), *ViewTarget->GetName());
    USceneComponent* Root = ViewTarget->GetRootComponent();

    if (!Root)
        return;

    UE_LOG(LogTemp, Warning, TEXT("Root: %s"), *Root->GetName());

    TArray<UCameraComponent*> Cameras;
    ViewTarget->GetComponents<UCameraComponent>(Cameras, true);

    for (UCameraComponent* Camera : Cameras)
    {
        UE_LOG(LogTemp, Warning, TEXT("Camera : %s / Active : %s / Parent : %s"), *Camera->GetName(), Camera->IsActive() ? TEXT("true") : TEXT("false"), *GetNameSafe(Camera->GetAttachParent()));
    }


    const FVector Location = PC->PlayerCameraManager->GetCameraLocation();
    const FRotator Rotation = PC->PlayerCameraManager->GetCameraRotation();

    UE_LOG(LogTemp, Warning, TEXT("Camera Location : %s"), *Location.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Camera Rotation : %s"), *Rotation.ToString());
}