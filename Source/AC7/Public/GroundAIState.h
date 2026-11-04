// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIPilot.h"
#include "Defined.h"

/**
 * 
 */
class AC7_API GroundAIState : public AIState
{
public:
	GroundAIState();
	virtual ~GroundAIState();

	struct AircraftDefaultData : public DefaultDataBlock
	{
		TeamData* SquadData{ nullptr };
		TWeakObjectPtr<class AGroundVehicle> Owner{ nullptr };
	};

	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) { NextState = State::END; return SquadData != nullptr && Owner != nullptr && Radar != nullptr; }
	virtual void SetDefaultData(DefaultDataBlock* data) override;


	//virtual void SetOverrideData(TSharedPtr<DataSetterBlock>& data) = 0;

protected:
	TeamData* SquadData{ nullptr };
	TWeakObjectPtr<class AGroundVehicle> Owner{ nullptr };
	TWeakObjectPtr<class URadar> Radar{ nullptr };

	constexpr static double NearLength = ConvertFromAltitude(500.0);
	constexpr static double CloseLength = ConvertFromAltitude(1000.0);
	constexpr static double RangeLength = ConvertFromAltitude(5000.0);
	constexpr static double NearLengthSquare = NearLength * NearLength;
	constexpr static double CloseLengthSquare = CloseLength * CloseLength;
	constexpr static double RangeLengthSquare = RangeLength * RangeLength;
	constexpr static double MinimumAltitude = ConvertFromAltitude(2000.0);
	constexpr static double MaximumAltitude = ConvertFromAltitude(36000.0);
	constexpr static double AscendAltitude = ConvertFromAltitude(4000.0);
	constexpr static double DescendAltitude = ConvertFromAltitude(32000.0);
	constexpr static float steepBankAngle = 90.0f;
	constexpr static float bankAngle = 30.0f;

};

class AC7_API AttackState : public GroundAIState
{
public:
	AttackState() = default;
	virtual ~AttackState() = default;

	struct AttackStateData : public DataSetterBlock
	{
		TWeakObjectPtr<class AVehicleBase> SelectedTarget;
	};

	virtual bool Update() override;
	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) override;
	virtual void SetDefaultData(DefaultDataBlock* data) override;
	//virtual void SetOverrideData(TSharedPtr<DataSetterBlock>& data) override;

	void ChangeToHold();
	void ShootMissile();
	void ShootGun();

private:
	DECLARE_MULTICAST_DELEGATE(FVoidFunction)
	FVoidFunction HandleWeapon;

	TWeakObjectPtr<class UFireControlSystem> FCS;
	FTargetInfo* Target;
	TArray<FTargetInfo>* Targets;
	
	float AttackTimer{ 0.0f };
	static constexpr float AttackFrequency{ 1.0f };
	double GunRange{ ConvertFromDistance(1000.0) };
	double MaxRange{ ConvertFromDistance(1000.0) };
	double SquaredLength;

	const float* TargetYaw{ nullptr },* TargetPitch{ nullptr };
	const float* TurretYaw{ nullptr },* TurretPitch{ nullptr };
};

class AC7_API HoldState : public GroundAIState
{
public:
	HoldState() = default;
	virtual ~HoldState() = default;

	struct SearchData : public DataSetterBlock
	{
		bool bSearching{ false };
	};

	inline virtual bool Update() override;
	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) override;
	//virtual void SetOverrideData(TSharedPtr<DataSetterBlock>& data) override;

	void StartSearch();
	bool SearchInRadar();
private:
	TWeakObjectPtr<class AVehicleBase> SelectedTarget;
	UE::Tasks::TTask<void> TaskHandle;
};
