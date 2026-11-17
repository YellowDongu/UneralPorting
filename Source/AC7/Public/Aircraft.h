// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VehicleBase.h"
#include "Defined.h"

#include "Aircraft.generated.h"

/**
 *
 */

/**
 * 
 */
UCLASS()
class AC7_API AAircraft : public AVehicleBase
{
	GENERATED_BODY()
public:
	AAircraft();
	virtual ~AAircraft() = default;

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	//virtual void OnConstruction();
	//virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	// 개체가 활성화될 때 세팅 작업
	virtual void SystemInitialize(const FSpawnInfomation& spawnInfomation) override;
	// 객체 생성 후 초기화작업, 다만 외부에서 실시하도록 함 -> 로딩 중이던 아니던 초기화 타이밍을 확실하게 하기 위해서
	virtual void SystemIntegration() override;
	// 카메라를 기체서 붙이고 그 결과를 컨트롤러에게 넘겨주는 형식, 이 녀석이 없어졌을 때 떼서 쓸 수 있는지 확인해봐야됨
	virtual void AttachCamera(TWeakObjectPtr<class UCameraComponent>& camera, bool thirdView = true) override;

	virtual void SetMoveable(bool active) override;

	//FireControlSystem
	inline void ChangeTarget(void);
	inline bool ShootMissile(TArray<int>& output);
	inline bool ChangeMissile(void);
	inline bool ShootMissile(void);
	inline bool ShootGun(void);
	inline void SetTarget(const TWeakObjectPtr<class AVehicleBase>& target);

	inline const TArray<FTargetInfo>& GetSelectedTargets(void) const;
	inline bool GetSelectSpecial(void) const;
	inline int GetSpecial(void) const;

	//Radar
	inline void StartDeployFlare();
	inline float GetRaderDistance() const;
	inline const TSet<TWeakObjectPtr<class AVehicleBase>>& GetInRadarTargets() const;

	inline void SetGear(bool value, bool instantSet = false);
	inline void DeployGear(bool instantSet = false);


	inline void GetControl(TObjectPtr<FControl>& poineter) { poineter = &Control; } // 공유용, 잘못 쓰지 말라고 포인터를 직접 넣게 함
	inline void GetControl(FControl*& poineter) { poineter = &Control; } // 공유용, 잘못 쓰지 말라고 포인터를 직접 넣게 함
	inline const FControl& GetControl() const { return Control; } // 참조용
	TWeakObjectPtr<class URadar> GetRadar() { return Radar; }
	TWeakObjectPtr<class UFlightMovement> GetFlightMovement() { return FlightMovement; }
	TWeakObjectPtr<class UFireControlSystem> GetFireControlSystem() { return FireControlSystem; }
	TWeakObjectPtr<class UAircraftAnimator> GetAircraftAnimator() { return AircraftAnimator; }
	float GetAirSpeed() { return Control.Velocity; }
	const FAircraftSpec& GetAircraftSpec() const { return SPEC; }

protected:
	FControl Control{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AircraftSpec", meta = (AllowPrivateAccess = "true"))
	FAircraftSpec SPEC;
	bool bSpringArmInitialized{ false }, bSystemIntegrated{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSetting", meta = (AllowPrivateAccess = "true"))
	float TargetArmLength{ 200.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSetting", meta = (AllowPrivateAccess = "true"))
	FVector SocketOffset{ FVector(0.0f, 0.0f, 50.0f) };

	FVector3d MeshCenter;
	TWeakObjectPtr<class URadar> Radar{ nullptr };
	TWeakObjectPtr<class UFlightMovement> FlightMovement{ nullptr };
	TWeakObjectPtr<class UAircraftAnimator> AircraftAnimator{ nullptr };
	TWeakObjectPtr<class UFireControlSystem> FireControlSystem{ nullptr };
	TWeakObjectPtr<class UAudioComponent> Audio{ nullptr };
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	//class UFlightMovement* flightMovement{ nullptr }; -> 근데 스마트포인터나 이거나 수명의 관점에서는 딱히 차이가 없어서 블루프린트에서 굳이 띄울 필요가 없는 녀석은 스마트 포인터로 빼는게 제일 나은 선택일까
};
