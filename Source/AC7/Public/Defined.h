// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Defined.generated.h"

#define MapEditMode

template<typename Type>
inline constexpr Type ConvertFromAltitude(Type value) { return value * 5.0; }
template<typename Type>
inline constexpr Type ConvertToAltitude(Type value) { return value / 5.0; }
template<typename Type>
inline constexpr Type ConvertFromVelocity(Type value) { return value * 3.0; } // velocity to real velocity 
template<typename Type>
inline constexpr Type ConvertToVelocity(Type value) { return value; } // velocity to visible speed number
template<typename Type>
inline constexpr Type ConvertFromDistance(Type value) { return value * 5.0; }
template<typename Type>
inline constexpr Type ConvertToDistance(Type value) { return value / 5.0; }

inline float CalibrateAngle(float angle) { return FMath::UnwindDegrees(angle); } // 메모용
inline double CalibrateAngle(double angle) { return FMath::UnwindDegrees(angle); } // 메모용

FString ExtractFileName(const FString& FilePath); // 메모용
FString ExtractFileExtension(const FString& FilePath); // 메모용
FString ExtractFileFullName(const FString& FilePath); // 메모용

struct FTargetInfo
{
	FTargetInfo() = default;
	FTargetInfo(const TWeakObjectPtr<class AVehicleBase>& newTarget) : Target(newTarget) {}
		
	inline void LockReset(void) { Lock = 1.0f; }
	inline const bool IsLock(void) const { return Lock < 0.001f; }
	void CalculateDistance(const TWeakObjectPtr<class AVehicleBase>& onwer, bool OnlySqared = true);
	void CalculateDistance(const FVector& position,bool isWorldPosition = true, bool OnlySqared = true);
	void CalculateDistance(double distanceSquared, bool OnlySqared = true) { DistanceSquared = distanceSquared; if (!OnlySqared) Distance = FMath::Sqrt(DistanceSquared); }

	TWeakObjectPtr<class AVehicleBase> Target{ nullptr };
	float Key{ 0.0f }, Lock{ 1.0f };
	double DistanceSquared{ 0.0 }, Distance{ 0.0 };
};

USTRUCT(BlueprintType)
struct FAircraftSpec
{
	//GENERATED_USTRUCT_BODY()
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSpec", meta = (AllowPrivateAccess = "true"))
	float GunRPM{ 350.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarSpec", meta = (AllowPrivateAccess = "true"))
	float RaderDistance{ ConvertFromDistance(10000.0f) };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlareSpec", meta = (AllowPrivateAccess = "true"))
	int FlareCount{ 2 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlareSpec", meta = (AllowPrivateAccess = "true"))
	float FlareCoolTime{ 50.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlareSpec", meta = (AllowPrivateAccess = "true"))
	float FlareRPM{ 550.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlareSpec", meta = (AllowPrivateAccess = "true"))
	int DeployCount{ 15 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightSpec", meta = (AllowPrivateAccess = "true"))
	float NonExceedSpeed{ 180.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightSpec", meta = (AllowPrivateAccess = "true"))
	float IdleSpeed{ 33.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightSpec", meta = (AllowPrivateAccess = "true"))
	float EnginePower{ 50.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightSpec", meta = (AllowPrivateAccess = "true"))
	float AirbrakePower{ 1.2f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightSpec", meta = (AllowPrivateAccess = "true"))
	FVector DragFactor = FVector::OneVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightSpec", meta = (AllowPrivateAccess = "true"))
	FVector RotationSpeed = FVector::OneVector;
};

UENUM(BlueprintType)
//enum class EVehicleID
enum class EVehicleID : uint8
{
	Aircraft = 0,
	F15E,
	SU33,
	F14D,

	AirEND,
	Ground = 100,
	GECK,
	GEPA,

	GroundEND,
	END
};

UENUM(BlueprintType)
enum class EWeaponID : uint8
{
	NULLID = 0,
	Standard,
	FAAM,
	SAAM,
	UGB,
	LGB,
	END
};


inline FString WeaponIDToString(EWeaponID id)
{
	switch (id)
	{
	case EWeaponID::Standard:
		return FString(TEXT("Standard"));
	case EWeaponID::FAAM:
		return FString(TEXT("FAAM"));
	case EWeaponID::SAAM:
		return FString(TEXT("SAAM"));
	case EWeaponID::UGB:
		return FString(TEXT("UGB"));
	case EWeaponID::LGB:
		return FString(TEXT("LGB"));
	default:
		return FString(TEXT(""));
	}

	return FString(TEXT(""));
};



USTRUCT(BlueprintType)
struct FWeaponMask
{
	GENERATED_BODY()

	const inline void Initialize()
	{
		if (mask == -1)
		{
			mask = 0;
			mask |= bLand ? 1 << 0 : 0;
			mask |= bSea ? 1 << 1 : 0;
			mask |= bAir ? 1 << 2 : 0;
		}
	}

	const inline bool MaskMatch(const FWeaponMask& otherMask) const { return MaskMatch(otherMask.mask); }
	const inline bool MaskMatch(int otherMask) const { return (mask & otherMask) != 0; }

	const bool IsLand(void) const { return mask & (1 << 0); }
	const bool IsSea(void) const { return mask & (1 << 1); }
	const bool IsAir(void) const { return mask & (1 << 2); }
	int GetMask(void) const { return mask; }
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bLand{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bSea{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bAir{ false };

	int mask{ -1 };
};

//USTRUCT()
struct FControl
{
	//GENERATED_USTRUCT_BODY()
public:
	FVector3f Yoke{ FVector3f::ZeroVector };
	float Throttle{ 0.2f };
	bool isAirBrakeOn{ false };
	bool isGearDown{ false };
	bool bActiveHighGTurn{ false };
	float Velocity = 33.0f;

	float GetHighGTurn(float deltaTime)
	{
		if (bIsActiveHighG != bActiveHighGTurn)
		{
			bIsActiveHighG = bActiveHighGTurn;
			HighGTurn = bIsActiveHighG ? 5.0f : 1.0f;
		}

		HighGTurn = FMath::Max(HighGTurn - deltaTime * 2.0f, 1.0f);
		return HighGTurn;
	}

	//DECLARE_DELEGATE_OneParam(FCalculateThrottle, float);
	DECLARE_DELEGATE_RetVal_OneParam(float, FCalculateThrottle, float);
	FCalculateThrottle CalculateThrottle;

private:
	bool bIsActiveHighG{ false };
	float HighGTurn{ 1.0f };
};

USTRUCT(BlueprintType)
struct FPlayerSpawnInfomation
{
	GENERATED_BODY()
public:
	int SpecialWeaponIndex{ -1 };
	EVehicleID VehicleID{ EVehicleID::F15E };
};

USTRUCT(BlueprintType)
struct FSpawnInfomation
{
	GENERATED_BODY()
public:
	inline void Initialize(AActor* spawner)
	{
		if (SpawnNode == nullptr)
		{
			Rotation = spawner->GetActorRotation().Add(LocalRotation.Y, LocalRotation.Z, LocalRotation.X);
			Position = spawner->GetActorLocation() + LocalPosition;
		}
		else
		{
			Position = SpawnNode->GetActorLocation();
			Rotation = SpawnNode->GetActorRotation();
		}

	}

	const FVector& GlobalPosition() const { return Position; }
	const FRotator& GlobalRotation() const { return Rotation; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true"))
	bool bIsPlayer{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true"))
	int LeaderIndex{ -1 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true"))
	int TeamIndex{ 2 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnLocation", meta = (AllowPrivateAccess = "true"))
	FVector LocalPosition{ FVector::ZeroVector };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnLocation", meta = (AllowPrivateAccess = "true"))
	FVector LocalRotation{ FVector::ZeroVector };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnLocation", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AActor> SpawnNode{ nullptr };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	EVehicleID VehicleID{ EVehicleID::END };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	bool IsTGT{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	bool bIsInvincible{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	int SpecialWeapon{ false };

	const FPlayerSpawnInfomation* playerInfoPointer{ nullptr };

private:
	FVector Position;
	FRotator Rotation;
};

USTRUCT()
struct FStringListPanelController
{
	GENERATED_BODY()
public:
	FStringListPanelController() = default;
	FStringListPanelController(class UCanvasPanel* SelectedBar, class UCanvasPanel* ListCanvas, class UVerticalBox* ListPanel);
	void Initialize(class UCanvasPanel* SelectedBar, class UCanvasPanel* ListCanvas, class UVerticalBox* ListPanel);

	void ChangeIndex(int index);
	void ChangeChildsText(TFunction<void(TArray<TObjectPtr<class UTextBlock>>& TextBlockList, int& MaxChildCount)> childTextChangeFunction);

	int GetMaxIndex() { return MaxIndex; }
	int GetCurrentIndex() { return Index; }
	void SetActive(bool active) { bActive = active; }
	void SetXSize(float base, float height) { BaseYValue = base; CellYValue = height; }
private:
	void Initialize();

	bool bActive{ false };
	int MaxIndex{ 0 }, Index{ 0 };
	float BaseYValue{ 0.0f }, CellYValue{ 0.0f };

	TArray<TObjectPtr<class UTextBlock>> TextBlockList;
	UPROPERTY()
	class UCanvasPanel* SelectedBar{ nullptr };
	UPROPERTY()
	class UCanvasPanel* ListCanvas{ nullptr };
	UPROPERTY()
	class UVerticalBox* ListPanel{ nullptr };
};


struct TeamData
{
	void Reset(void);

	void ChangeSquadNumber(int value) { SquadNumber = value; Offset = GetLocalPositionOffset(SquadNumber); }

	FVector GetLocalPositionOffset(int index) { return FVector(static_cast<double>(index) * 0.5 * -SpaceInterval, static_cast<double>(index) * SpaceInterval * ((index % 2 == 0) ? 1.0 : -1.0), 0.0); }
	FVector GetOffset(void) { return Offset; }
	int GetSquadNumber(void) { return SquadNumber; }
	bool IsLeader(void) { return Leader != nullptr && *Leader == MyVehicle; }

	TWeakObjectPtr<class AVehicleBase>* Leader{ nullptr };
	TWeakObjectPtr<class AVehicleBase> MyVehicle{ nullptr };
	//int TeamIndex{ 0 };
private:
	FVector Offset;
	int SquadNumber{ 0 };
	static constexpr double SpaceInterval{ ConvertFromDistance(100.0) };
};

USTRUCT(BlueprintType)
struct FWaypointData
{
	GENERATED_BODY()
public:
	FWaypointData() = default;
	FWaypointData(FVector position, FQuat rotation, double distanceValue) : Position(position), Rotation(rotation), DistanceValue(distanceValue) {}
	~FWaypointData() = default;

	UPROPERTY(EditAnywhere) FVector Position { FVector::ZeroVector };
	UPROPERTY(EditAnywhere) FQuat Rotation { FQuat::Identity };
	UPROPERTY(EditAnywhere) double DistanceValue{ 0.0 }; // 이전 점과의 거리, 없으면 0
	//UPROPERTY(EditAnywhere) bool bAutoRotation = false;   // 쓰기로 하면
};