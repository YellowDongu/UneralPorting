// Fill out your copyright notice in the Description page of Project Settings.


#include "MovementPathAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "FileHelpers.h"
#include "GameMaster.h"
#include "BluePrintCollector.h"

UMovementPathAsset* UMovementPathAsset::GetOrCreateAsset(const FString& PackageName)
{
    FString AssetName = FPackageName::GetLongPackageAssetName(PackageName);
    FString ObjectPath = PackageName + TEXT(".") + AssetName;

    if (UMovementPathAsset* Existing = LoadObject<UMovementPathAsset>(nullptr, *ObjectPath, nullptr, LOAD_NoWarn | LOAD_Quiet)) // 메모리에 있든 디스크에만 있든 찾는다 (없을 때 경고 로그가 안 뜨게 플래그 지정)
        return Existing;

    UPackage* Package = CreatePackage(*PackageName);
    UMovementPathAsset* Asset = NewObject<UMovementPathAsset>(Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);
    FAssetRegistryModule::AssetCreated(Asset);
    return Asset;
}

UMovementPathAsset* UMovementPathAsset::Load(const FString& ObjectPath)   // 예: "/Game/EditorOnly/Waypoints/Path01.Path01"
{
    return LoadObject<UMovementPathAsset>(nullptr, *ObjectPath);
}

void UMovementPathAsset::Load(UWorld& world, const FString& name, TArray<FWaypointData>& output)
{
    output.Empty();

    UE_LOG(LogTemp, Error, TEXT("[UMovementPathAsset::Load] Load Asset : %s"), *name);
    UBluePrintCollector* collector = world.GetGameInstance<UGameMaster>()->GetCollector();
    check(collector);

    const TSoftObjectPtr<class UMovementPathAsset>* asset = collector->GetManeuverAsset(name);
    if (asset == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[UMovementPathAsset::Load] Asset Not Found : %s"), *name);
        return;
    }

    UMovementPathAsset* loadedAsset = UAssetManager::GetStreamableManager().LoadSynchronous(*asset);
    if (loadedAsset == nullptr)
    {
        //loadedAsset = LoadObject<UMovementPathAsset>(nullptr, *asset->ToString());
        //if (loadedAsset == nullptr)
        UE_LOG(LogTemp, Error, TEXT("[UMovementPathAsset::Load] Asset Not Loaded : %s"), *name);
            return;
    }
    
    output = MoveTemp(loadedAsset->SavedPoints);
}
