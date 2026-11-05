// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIPilot.h"
#include "Defined.h"
/**
 * 
 */

class AC7_API AIAircraftState : public AIState
{
public:
	AIAircraftState();
	virtual ~AIAircraftState();

	struct AircraftDefaultData : public DefaultDataBlock
	{
		TeamData* SquadData{ nullptr };
		FControl* Control{ nullptr };
		TWeakObjectPtr<class AAircraft> Owner{ nullptr };
	};

	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr);
	virtual void SetDefaultData(DefaultDataBlock* data) override;

	bool HorizontalRotation(const FVector& targetPosition, double* leftSquaredLength = nullptr);

	bool Leveling();
	bool SetAttitude(float targetPitch, float targetRoll, float targetYaw = 0.0f);
	void SteepTurn(FRotator& aircraftRotation, float yaw, float pitch);
	void ShallowTurn(FRotator& aircraftRotation, float yaw, float pitch);
	bool YawRotation(FRotator& aircraftRotation, float yaw, float pitch);
	bool StraightFollow(FRotator& aircraftRotation, FVector& aircraftDirection, float yaw, float pitch);
	void RearFollow(FRotator& aircraftRotation, FRotator& targetRotation, float pitch);

	bool DefendAltitude(TSharedPtr<DataSetterBlock>& output);
	bool NoticeWarning(TSharedPtr<DataSetterBlock>& output);
	bool CogitateCombat();
	bool CogitateInFormation();

protected:
	TeamData* SquadData{ nullptr };
	FControl* Control{ nullptr };
	TWeakObjectPtr<class AAircraft> Owner{ nullptr };
	TWeakObjectPtr<class URadar> Radar{ nullptr };
	TWeakObjectPtr<class UTeamManagementSystem> TeamSystem;

	constexpr static double NearLength = ConvertFromDistance(500.0);
	constexpr static double CloseLength = ConvertFromDistance(1000.0);
	constexpr static double RangeLength = ConvertFromDistance(6000.0);
	constexpr static double NearLengthSquare = NearLength * NearLength;
	constexpr static double CloseLengthSquare = CloseLength * CloseLength;
	constexpr static double RangeLengthSquare = RangeLength * RangeLength;
	constexpr static double MinimumAltitude = ConvertFromAltitude(2000.0);
	constexpr static double MaximumAltitude = ConvertFromAltitude(36000.0);
	constexpr static double AscendAltitude = ConvertFromAltitude(3000.0);
	constexpr static double DescendAltitude = ConvertFromAltitude(34000.0);
	constexpr static float steepBankAngle = 90.0f;
	constexpr static float bankAngle = 30.0f;
};

class AC7_API CombatState : public AIAircraftState
{
public:
	CombatState() = default;
	virtual ~CombatState() = default;

	struct CombatStateDefaultData : public DataSetterBlock
	{
		TWeakObjectPtr<class AVehicleBase> SelectedTarget;
	};

	virtual bool Update() override;
	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) override;
	virtual void SetDefaultData(DefaultDataBlock* data) override;
	virtual TSharedPtr<OverridedDataBlock> SetOverrideData(void) override;

	void ChangeToChase();
private:
	TWeakObjectPtr<class AVehicleBase> Target;

	float AttackTimer{ 0.0f };
	static constexpr float AttackFrequency{ 1.0f };
};

class AC7_API ChaseState : public AIAircraftState
{
public:
	struct ChaseStateDefaultData : public DataSetterBlock
	{
		TWeakObjectPtr<class AVehicleBase> SelectedTarget;
	};

	ChaseState() = default;
	virtual ~ChaseState() = default;

	virtual bool Update() override;
	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) override;
	virtual TSharedPtr<OverridedDataBlock> SetOverrideData(void) override;

	void ChangeToCombat();
	//virtual void SetDefaultData(DefaultDataBlock* data) override { AIAircraftState::SetDefaultData(data); }

private:
	TWeakObjectPtr<class AVehicleBase> ChasingTarget;
};

class AC7_API SearchState : public AIAircraftState
{
public:
	SearchState() = default;
	virtual ~SearchState() = default;

	struct SearchData : public DataSetterBlock
	{
		bool bSearching{ false };
	};

	inline virtual bool Update() override;
	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) override;
	virtual TSharedPtr<OverridedDataBlock> SetOverrideData(void) override;

	void StartSearch();
	bool SearchInRadar();
private:
	TWeakObjectPtr<class AVehicleBase> SelectedTarget;
	UE::Tasks::TTask<void> TaskHandle;
};

class AC7_API InFomationState : public AIAircraftState
{
public:
	InFomationState() = default;
	virtual ~InFomationState() = default;

	virtual bool Update() override;
	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) override;
	virtual TSharedPtr<OverridedDataBlock> SetOverrideData(void) override;
	//virtual void SetDefaultData(DefaultDataBlock* data) override { AIAircraftState::SetDefaultData(data); }
private:
};

class AC7_API AltitudeSetterState : public AIAircraftState
{
public:
	AltitudeSetterState() = default;
	virtual ~AltitudeSetterState() = default;

	struct AltitudeSetterData : public OverridedDataBlock
	{
		double TargetAltitude;
	};

	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) override;
	virtual bool Update() override;
	//virtual void SetDefaultData(DefaultDataBlock* data) override { AIAircraftState::SetDefaultData(data); }

private:
	double TargetAltitude;

};

class AC7_API ManeuverState : public AIAircraftState
{
public:
	ManeuverState() = default;
	virtual ~ManeuverState() { Release(); }

	struct ManeuverData : public DataSetterBlock { FString ManeuverName; };

	virtual void Release(void) override;
	virtual bool Update(void) override;
	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) override;
	
	static float GetRotationTime(const FQuat& StartQuat, const FQuat& TargetQuat, const FVector& AngularSpeed);
	bool InitializeData(void);
	int CheckWayCollision(void);

	void SetAviliableManeuver(const TArray<FString>& data) { AviliableManeuver.Reset(); AviliableManeuver.Append(data); }
	void AddAviliableManeuver(const TArray<FString>& data) { AviliableManeuver.Append(data); }
	void AddAviliableManeuver(const FString& name) { AviliableManeuver.Add(name); }
protected:
	int Phase{ 0 }, CollisionCheck{ 0 };
	float Radius{ 100.0f }, Timer{ 1.0f };

	TArray<FString> AviliableManeuver;
	TArray<FWaypointData> WaypointData;
	TArray<FTraceHandle> HandleList;
	UE::Tasks::TTask<void> Handle;

	FVector InitialPosition;
	FRotator InitialRotation;
	FQuat YawQuaternion;
};


class AC7_API DefendState : public ManeuverState
{
public:
	DefendState() = default;
	virtual ~DefendState() { Release(); }

	virtual bool Update() override;
	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) override;
	virtual TSharedPtr<OverridedDataBlock> SetOverrideData(void) override;

	void Initiate();
private:
	FRotator TargetRotation;
	bool ReturnFlag{ true };
	float Timer{ DelayTime };
	double Random{ 0.0 };
	static constexpr float DelayTime{ 5.0f };
	static constexpr double AltitudeMiddleValue{ (MaximumAltitude + MinimumAltitude) * 0.5 };
};
