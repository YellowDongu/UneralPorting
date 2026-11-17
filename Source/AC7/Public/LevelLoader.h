// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"


#include "Engine/StreamableManager.h"

#include "LevelLoader.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AC7_API ULevelLoader : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void ChangeLevel(const TSoftObjectPtr<UWorld>& level) { ChangeLevel(FName(level.ToSoftObjectPath().GetAssetName())); }
	void ChangeLevel(const FName& name);

	static bool GetAssetNeeded(const FName& targetPath, TSet<FSoftObjectPath>& assetsToLoad) { return GetAssetNeeded(FSoftObjectPath(targetPath.ToString()), assetsToLoad); }
	static bool GetAssetNeeded(const FSoftObjectPath& targetPath, TSet<FSoftObjectPath>& assetsToLoad);
	static TMap<FString, TSoftObjectPtr<USoundBase>> GetAllSoundAsset(void);
	static TMap<FString, TSoftObjectPtr<class UMovementPathAsset>> GetAllWaypointAsset(void);
	
	void AssetSyncLoad(const FString& handleName, const TArray<FSoftObjectPath>& pathList, bool override, const FStreamableDelegate& delegate);
	void AssetSyncLoad(const FString& handleName, const FSoftObjectPath& targetPath, bool override = true, const FStreamableDelegate& delegate = FStreamableDelegate()) { return AssetSyncLoad(handleName, TArray<FSoftObjectPath>{ targetPath }, override, delegate); }
	void AssetAsyncLoad(const FString& handleName, const TArray<FSoftObjectPath>& targetPath, bool override = true, const FStreamableDelegate& delegate = FStreamableDelegate());
	void AssetAsyncLoad(const FString& handleName, const FSoftObjectPath& targetPath, bool override = true, const FStreamableDelegate& delegate = FStreamableDelegate()) { return AssetAsyncLoad(handleName, TArray<FSoftObjectPath>{ targetPath }, override, delegate); }

	inline void ActiveChangeTrigger() { bActiveChangeTrigger = true; }
	void Unload(const FString& Name) { if (Name.IsEmpty() || !LoadedAssetHandles.Contains(Name)) return; LoadedAssetHandles.Remove(Name); }

	void LevelReserve(const FString& value) { ReservedLevel = FName(TEXT("/Game/Level/") + value + TEXT(".") + value); }
	const FName& GetReservedLevelPath(void) const { return ReservedLevel; }
	FName GetCurrentLevel(void) { return CurrentLevel; }

private:
	FName EmptyLevelName{ TEXT("Empty") };
	FName ReservedLevel, CurrentLevel;

	TMap<FString, TSharedPtr<FStreamableHandle>> LoadedAssetHandles;
	TMap<FString, TSoftObjectPtr<USoundBase>> Sounds;
	
	FTimerHandle CheckTimer;
	FDelegateHandle DelegateHandle;
	bool bLoading = false;
	bool bActiveChangeTrigger = false;
};
