// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Defined.h"
#include "VehicleBase.generated.h"

UCLASS(Abstract)
class AC7_API AVehicleBase : public APawn
{
    //===========================================
    // Initializer/Destructor
    //===========================================

	GENERATED_BODY()
public:
	AVehicleBase();
	//virtual ~AVehicleBase() = default;

    //virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void AttachCamera(TWeakObjectPtr<class UCameraComponent>& camera, bool thirdView = true);
    virtual void SystemInitialize(const FSpawnInfomation& spawnInfomation);
    virtual void SystemIntegration() PURE_VIRTUAL(AVehicleBase::SystemIntegration, );
    //===========================================
    // Methods
    //===========================================

	virtual void Tick(float DeltaTime) override;
    virtual void Release(void);
    virtual void SetMoveable(bool active) { bMoveable = active; } // pure?
    void HPDamage(int damage);


    //public delegate void ReleaseMethod(Vehicle vehicle);
    //public event ReleaseMethod release;
    //public delegate void ReleaseEventMethod();
    //public event ReleaseEventMethod releaseEvent;
    //public delegate void SetIntValueMethod(int value);
    //public event SetIntValueMethod SetIntValue;

    //public void SetRelease(ReleaseMethod listener) { release = listener; }
    //public VehicleID ID { get { return id; } private set { id = value; } }

    //===========================================
    // Variable & GetSet Methods
    //===========================================

    TeamData& GetSquadData(void) { return SquadSystem; }
    void SquadDataReset(void) { SquadSystem.Reset(); SquadSystem.MyVehicle = this; }
    EVehicleID GetVehicleID(void) const { return id; }
    FString GetVehicleName(void) const { return vehicleName; }
    int GetTeam(void) const { return Team; }
    bool IsTGT(void) const { return bIsTGT; }
    bool IsLand(void) const { return VehicleMask.IsLand(); }
    bool IsAir(void) const { return VehicleMask.IsAir(); }
    bool IsAlive(void) const { return bIsAlive; }
    const FWeaponMask& GetMask(void) const { return VehicleMask; }
    float HPPresentage(void) const { return 1.0f - (float)HP / (float)MaxHp; }
    int GetHP(void) const { return HP; }
    int GetMaxHp(void) const { return MaxHp; }

    class USceneComponent* GetFirstCameraArm() { return FirstCameraArm; }
    class USpringArmComponent* GetThirdCameraArm() { return ThirdCameraArm; }

    DECLARE_MULTICAST_DELEGATE_OneParam(FOnDamageEvent, AVehicleBase*);
	inline FDelegateHandle AddDamageEvent(const FOnDamageEvent::FDelegate& InDelegate) { return DamageEvent.Add(InDelegate); }
	inline bool RemoveDamageEvent(const FDelegateHandle& handle) { return DamageEvent.Remove(handle); }
protected:
    FOnDamageEvent DamageEvent;

    bool bMoveable{ false };
    bool bIsTGT{ false }, bIsAlive{ false };
    int HP{ 0 };
    int Team{ 0 };

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property", meta = (AllowPrivateAccess = "true"))
    int MaxHp{ 0 };

    UPROPERTY()
    class USceneComponent* FirstCameraArm{ nullptr };
    UPROPERTY()
    class USpringArmComponent* ThirdCameraArm{ nullptr };

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property", meta = (AllowPrivateAccess = "true"))
    FWeaponMask VehicleMask;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property", meta = (AllowPrivateAccess = "true"))
    EVehicleID id{ EVehicleID::END };
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property", meta = (AllowPrivateAccess = "true"))
    FString vehicleName{ TEXT("NULL") };

    TeamData SquadSystem;
};


inline bool IsAlive(const TWeakObjectPtr<AVehicleBase>& object) { return object.IsValid() && object->IsAlive(); }
inline bool IsAlive(const TObjectPtr<AVehicleBase>& object) { return IsValid(object) && object->IsAlive(); }
inline bool IsAlive(const AVehicleBase* object) { return IsValid(object) && object->IsAlive(); }
