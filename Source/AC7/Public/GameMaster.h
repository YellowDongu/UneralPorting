// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Defined.h"
#include "GameMaster.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UGameMaster : public UGameInstance
{
	GENERATED_BODY()
public:
	UGameMaster();

	virtual void Init(void) override;
	//virtual void Shutdown(void) override;
	virtual void OnStart(void) override;
	virtual void Shutdown(void) override;

	FString GetMissionName(int index);
	TSoftObjectPtr<UWorld> GetReserveLevel(void) { return ReservedLevel; }
	void ReserveLevel(const FString& nextLevel);
	void ReserveLevel(const TSoftObjectPtr<UWorld>& nextLevel) { ReservedLevel = nextLevel; }
	void CancelReservationLevel(void) { ReservedLevel = nullptr; }
	TSoftObjectPtr<UWorld> FindMissionLevel(int index);
	TSoftObjectPtr<UWorld> FindMissionLevel(const FString& name);
	void ChangeLevel(const FName& levelName);
	void ChangeLevel(const TSoftObjectPtr<UWorld>& level) { ChangeLevel(FName(level.ToSoftObjectPath().GetAssetName())); }
	void ChangeReservedLevel(void) { ChangeLevel(FName(ReservedLevel.ToSoftObjectPath().GetAssetName())); ReservedLevel = nullptr; }

	void SetPlayerInfomation(const FPlayerSpawnInfomation& infomation) { PlayerSpawnInfomation = infomation; }
	const FPlayerSpawnInfomation& GetPlayerInfomation(void) const { return PlayerSpawnInfomation; }

	USoundBase* GetSoundBluePrint(const FString& bgmName);
	inline const TObjectPtr<class UBluePrintCollector>& GetCollector();

	UFUNCTION(Exec)
	void DebugCamera();

private:
	FPlayerSpawnInfomation PlayerSpawnInfomation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UBluePrintCollector> CollectorBluePrint;
	UPROPERTY() TObjectPtr<class UBluePrintCollector> Collector;

	TSoftObjectPtr<UWorld> ReservedLevel{ nullptr };
};
