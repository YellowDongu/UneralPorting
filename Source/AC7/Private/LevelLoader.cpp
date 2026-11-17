// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelLoader.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Tasks/Task.h"
#include "Async/TaskGraphInterfaces.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "MovementPathAsset.h"

//#include "UObject/UObjectGlobals.h"
//void ULevelLoader::Deinitialize()
//{
//    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
//    Super::Deinitialize();
//}

TMap<FString, TSoftObjectPtr<USoundBase>> ULevelLoader::GetAllSoundAsset()
{
    IAssetRegistry& assetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    TMap<FString, TSoftObjectPtr<USoundBase>> result;

    FARFilter filter;
    filter.ClassPaths.Add(USoundBase::StaticClass()->GetClassPathName());
    filter.bRecursiveClasses = true;

    TArray<FAssetData> foundedAssets;
    assetRegistry.GetAssets(filter, foundedAssets);
    result.Reserve(foundedAssets.Num());

    for (const FAssetData& Asset : foundedAssets)
    {
        TSoftObjectPtr<USoundBase> Sound(Asset.ToSoftObjectPath());
        result.Add(Asset.AssetName.ToString(), MoveTemp(Sound));
    }

    return result;
}

#define RemoveTestingCase
TMap<FString, TSoftObjectPtr<UMovementPathAsset>> ULevelLoader::GetAllWaypointAsset()
{
    IAssetRegistry& assetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    TMap<FString, TSoftObjectPtr<UMovementPathAsset>> result;

    FARFilter filter;
    filter.ClassPaths.Add(UMovementPathAsset::StaticClass()->GetClassPathName());
    //filter.PackagePaths.Add(FName(TEXT("Game/Vehicles/Aircrafts/AircraftManeuverPattern")));
    filter.bRecursiveClasses = true;

    TArray<FAssetData> foundedAssets;
    assetRegistry.GetAssets(filter, foundedAssets);
    result.Reserve(foundedAssets.Num());

    for (const FAssetData& Asset : foundedAssets)
#ifdef RemoveTestingCase
    {
        FString name = Asset.AssetName.ToString();
        if (name.Contains(TEXT("test"), ESearchCase::IgnoreCase) || name.Contains(TEXT("save"), ESearchCase::IgnoreCase))
            continue;
        result.Emplace(name, Asset.ToSoftObjectPath());
    }
#else
        result.Emplace(Asset.AssetName.ToString(), Asset.ToSoftObjectPath());
#endif

    return result;
}

bool ULevelLoader::GetAssetNeeded(const FSoftObjectPath& path, TSet<FSoftObjectPath>& assetsToLoad)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    //FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    const FAssetData assetData = AssetRegistry.GetAssetByObjectPath(path, true);

    if (!assetData.IsValid())
        return false;

    //UE::AssetRegistry::FDependencyQuery Query;
    //Query.Required = UE::AssetRegistry::EDependencyProperty::Hard;

    TQueue<FName> queue;
    TSet<FName> visited;

    assetsToLoad.Add(assetData.ToSoftObjectPath());
    queue.Enqueue(assetData.PackageName);
    visited.Add(assetData.PackageName);

    FName PackageName;
    TArray<FName> Dependencies;
    TArray<FAssetData> assets;

    while (!queue.IsEmpty())
    {
        if (!queue.Dequeue(PackageName))
            continue;

        Dependencies.Empty();
        //if (!AssetRegistry.GetDependencies(PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::Package, Query))
        if (!AssetRegistry.GetDependencies(PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::Package))
            continue;

        for (const FName& name : Dependencies)
        {
            assets.Empty();
            if (!AssetRegistry.GetAssetsByPackageName(name, assets, true, false))
                continue;

            for (const FAssetData& asset : assets)
                if (asset.IsValid())
                    assetsToLoad.Add(asset.ToSoftObjectPath());

            if (visited.Contains(name))
                continue;

            visited.Add(name);
            queue.Enqueue(name);
        }
    }

    return !assetsToLoad.IsEmpty();
}


void ULevelLoader::AssetSyncLoad(const FString& handleName, const TArray<FSoftObjectPath>& pathList, bool override, const FStreamableDelegate& delegate)
{
    if (pathList.IsEmpty())
        return;

    TSharedPtr<FStreamableHandle>& handle = LoadedAssetHandles.FindOrAdd(handleName);
    if (!override && handle.IsValid())
        return;

    handle.Reset();
    TSet<FSoftObjectPath> assetsToLoad;
    
    for (auto& item : pathList)
        GetAssetNeeded(item, assetsToLoad);

    if (IsInGameThread())
        handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(assetsToLoad.Array(), delegate);
    else
    {
        handle.Reset();
        AsyncTask(ENamedThreads::GameThread, [assetsToLoad, handle, delegate]()
                  {
                      UAssetManager::GetStreamableManager().RequestAsyncLoad(assetsToLoad.Array(), delegate);
                  });
    }
}

void ULevelLoader::AssetAsyncLoad(const FString& handleName, const TArray<FSoftObjectPath>& pathList, bool override, const FStreamableDelegate& delegate)
{
    if (pathList.IsEmpty())
        return;

    TSharedPtr<FStreamableHandle>* handle = &LoadedAssetHandles.FindOrAdd(handleName);
    if (!override && handle->IsValid())
        return;

    handle->Reset();

    FString taskName = TEXT("[ULevelLoader::PreLoad]") + handleName;
    UE::Tasks::Launch(*taskName, [this, pathList, handleName, delegate]()
                      {
                          TSet<FSoftObjectPath> assetsToLoad;
                          for (auto& item : pathList)
                              GetAssetNeeded(item, assetsToLoad);

                          if (assetsToLoad.IsEmpty())
                          {
                              delegate.Execute();
                              return;
                          }

                          AsyncTask(ENamedThreads::GameThread, [this, assetsToLoad, handleName, delegate]()
                                    {
                                        TSharedPtr<FStreamableHandle>& validHandle = LoadedAssetHandles.FindOrAdd(handleName);
                                        validHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(assetsToLoad.Array(), delegate);
                                    });
                      });

//    TSet<FSoftObjectPath> assetsToLoad;
//    if (!GetAssetNeeded(targetPath, assetsToLoad))
//        return nullptr;
//
//    if (IsInGameThread())
//        return UAssetManager::GetStreamableManager().RequestAsyncLoad(assetsToLoad.Array(), delegate);
//
//    TSharedPtr<FStreamableHandle> result;
//    FGraphEventRef loadTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
//        [&result, &assetsToLoad, &delegate]()
//        {
//            result = UAssetManager::GetStreamableManager().RequestAsyncLoad(assetsToLoad.Array(), delegate);
//        },
//        TStatId(), nullptr, ENamedThreads::GameThread);
//
//    FTaskGraphInterface::Get().WaitUntilTaskCompletes(loadTask);
//    return result;
}

void ULevelLoader::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CurrentLevel = FName(GetWorld()->GetMapName());
	UE_LOG(LogTemp, Log, TEXT("[ULevelLoader::Initialize] CurrentLevel: %s"), *CurrentLevel.ToString());
}

void ULevelLoader::ChangeLevel(const FName& TargetLevel)
{
    check(GetWorld());
    CurrentLevel = TargetLevel;
    UGameplayStatics::OpenLevel(GetWorld(), TargetLevel);
}
