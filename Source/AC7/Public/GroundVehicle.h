// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VehicleBase.h"
#include "GroundVehicle.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API AGroundVehicle : public AVehicleBase
{
	GENERATED_BODY()
public:
	AGroundVehicle();
	virtual ~AGroundVehicle() = default;

	virtual void BeginPlay() override;
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	//virtual void OnConstruction();
	//virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	// 개체가 활성화될 때 세팅 작업
	virtual void SystemInitialize(const FSpawnInfomation& spawnInfomation) override;
	// 객체 생성 후 초기화작업, 다만 외부에서 실시하도록 함 -> 로딩 중이던 아니던 초기화 타이밍을 확실하게 하기 위해서
	virtual void SystemIntegration() override;

	//FireControlSystem
	//inline void ChangeTarget();
	//inline bool ShootMissile(TArray<int>& output);
	//inline bool ChangeMissile();
	//inline bool ShootMissile();
	//inline bool ShootGun();
	//inline void SetTarget(const TWeakObjectPtr<class AVehicleBase>& target);
	//inline const TArray<FTargetInfo>& GetSelectedTargets() const;

	//Radar
	inline float GetRaderDistance() const;
	inline const TSet<TWeakObjectPtr<class AVehicleBase>>& GetInRadarTargets() const;


	TWeakObjectPtr<class URadar> GetRadar() { return Radar; }
	TWeakObjectPtr<class UFireControlSystem> GetFireControlSystem() { return FireControlSystem; }

protected:
	bool bSystemIntegrated{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	double RadarDistance{ 5000.0 };

	FVector3d MeshCenter;
	TWeakObjectPtr<class URadar> Radar{ nullptr };
	TWeakObjectPtr<class UFireControlSystem> FireControlSystem{ nullptr };
};
