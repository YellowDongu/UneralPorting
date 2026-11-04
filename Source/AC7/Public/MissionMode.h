// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MissionMode.generated.h"

/**
 * 
 */

UCLASS()
class AC7_API AMissionMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AMissionMode();

	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	//virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	//virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
	//virtual APlayerController* SpawnPlayerControllerCommon(ENetRole InRemoteRole, FVector const& SpawnLocation, FRotator const& SpawnRotation, TSubclassOf<APlayerController> InPlayerControllerClass) override;

	UFUNCTION(BlueprintCallable)
	void SetBoarder(const FVector& leftBottom, const FVector& rightTop);
	void TriggerWinLocal();
	void TriggerWinMission(class AUIHandler* hud); // 만약 멀티플레이면 모든 플레이어 상대로 이걸 돌리면 됨
	static bool GetSpectatorMode();
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APlayerController> PlayerController;
	class APlayerPilot* player;

	bool flag = false;
};
