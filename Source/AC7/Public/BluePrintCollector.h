// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Defined.h"
#include "BluePrintCollector.generated.h"

/**
 * 
 */
USTRUCT(Blueprintable)
struct FLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString LevelLabel;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> LevelPtr;
};

UCLASS(Blueprintable)
class AC7_API UBluePrintCollector : public UObject
{
	GENERATED_BODY()
	struct FVehicleInfo
	{
		FVehicleInfo(const TSubclassOf<class AVehicleBase>& vehicle);

		bool bIsLand{ false };
		EVehicleID VehicleID;
		FString VehicleName;
		FSoftObjectPath VehiclePath;
		TSubclassOf<class AVehicleBase> VehiclePtr;
	};

	struct FLevelInfo
	{
	public:
		FLevelInfo(void) = default;
		FLevelInfo(const TSoftObjectPtr<UWorld>& level);
		FString LevelName;
		FString LevelLabel;
		FSoftObjectPath LevelPath;
		TSoftObjectPtr<UWorld> LevelPtr;

		void SetData(const FLevelData& level)
		{
			LevelPtr = level.LevelPtr;
			LevelLabel = level.LevelLabel;
			LevelPath = LevelPtr.ToSoftObjectPath();
			LevelName = LevelPath.GetAssetName();
		}
		void SetInfo(const TSoftObjectPtr<UWorld>& level)
		{
			LevelPtr = level;
			LevelPath = LevelPtr.ToSoftObjectPath();
			LevelName = LevelPath.GetAssetName();
		}
	};

public:
	virtual void PostInitProperties(void) override;

	TWeakObjectPtr<class AVehicleBase> Create(EVehicleID ID, UWorld* world, bool& isAircraft);
	TWeakObjectPtr<class AAIPilot> CreateAIPilot(UWorld* world, const FSpawnInfomation& infomation);
	TWeakObjectPtr<class AFlare> CreateFlare(UWorld* world);
	TWeakObjectPtr<class ABullet> CreateBullet(UWorld* world);
	TObjectPtr<class UNiagaraSystem> CreateExplosionSystem(void);

	const TSoftObjectPtr<class UMovementPathAsset>* GetManeuverAsset(const FString& name) const { return ManeuverAssets.Find(name); }
	void GetManeuverData(const FString& name, TArray<FWaypointData>& output);

	FString GetMissionLevelName(int index);
	TSoftObjectPtr<UWorld> GetMissionLevel(int index) { return ((index < 0 || index >= Levels.Num()) ? nullptr : Levels[index]); }
	TSoftObjectPtr<UWorld> GetMissionLevel(const FString& Name) { auto result = LevelInfomation.Find(Name); return ((result == nullptr) ? nullptr : result->LevelPtr); }
	const FLevelInfo* GetMissionLevelData(const FString& Name) const { auto result = LevelInfomation.Find(Name); return ((result == nullptr) ? nullptr : result); }

	USoundBase* GetSound(const FString& name);
	const TSoftObjectPtr<UWorld>& GetSelectLevel() const { return SelectLevel; }
	const TSoftObjectPtr<UWorld>& GetMainMenuLevel() const { return MainMenuLevel; }
	TSubclassOf<class AVehicleBase> GetVehicleBluePrint(EVehicleID id) { const FVehicleInfo* result = VehicleInfomation.Find(id); return ((result == nullptr) ? nullptr : result->VehiclePtr); }

private:
	//BluePrintDefine(AAIAircraftPilot, AIAircraftPilotBluePrint);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Blueprint, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AAIAircraftPilot> AIAircraftPilotBluePrint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Blueprint, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AAIDriver> AIDriverPilot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Blueprint, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class AVehicleBase>> Vehicles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Blueprint, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AFlare> FlareBluePrint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Blueprint, meta = (AllowPrivateAccess = "true"))
	//TSubclassOf<class UNiagaraSystem> ExplosionBluePrint;
	UNiagaraSystem* ExplosionBluePrint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blueprint", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ABullet> BulletBluePrint;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LevelBlueprint, meta = (AllowPrivateAccess = "true"))
	TArray<FLevelData> LevelList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LevelBlueprint, meta = (AllowPrivateAccess = "true"))
	TArray<TSoftObjectPtr<UWorld>> Levels;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LevelBlueprint, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> SelectLevel;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LevelBlueprint, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> MainMenuLevel;

	TMap<EVehicleID, FVehicleInfo> VehicleInfomation;
	TMap<FString, FLevelInfo> LevelInfomation;
	TMap<FString, TObjectPtr<USoundBase>> Sounds;
	TMap<FString, TSoftObjectPtr<USoundBase>> SoundAssets;

	UPROPERTY(VisibleAnywhere)
	TMap<FString, TSoftObjectPtr<class UMovementPathAsset>> ManeuverAssets;
	TMap<FString, TArray<TArray<FWaypointData>>> LoadedManeuverData;
};
