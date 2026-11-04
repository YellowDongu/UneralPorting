// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Defined.h"
#include "FireControlSystem.generated.h"
//===========================================
// struct/enum
//===========================================
//FSTRUCT(BlueprintType)
USTRUCT(BlueprintType)
struct FSpecialMissileSlot
{
	GENERATED_BODY()
public:
    void Initialize(class USceneComponent* slotRoot, bool active);
    void SetActive(bool value);

    TWeakObjectPtr<class USceneComponent> Root;
	TArray<TWeakObjectPtr<class UStaticMeshComponent>> MeshSlot;
	TArray<TWeakObjectPtr<class AMissile>> MissileSlot;
private:
    bool bIsActive = false;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AC7_API UFireControlSystem : public UActorComponent
{
	GENERATED_BODY()

public:
    //===========================================
    // Initializer/Destructor
    //===========================================
	UFireControlSystem();

    bool WeaponIntegration(USceneComponent* WeaponComp);
    void SystemInitialize(int specialSelected);
    virtual void SystemIntegration(TWeakObjectPtr<class URadar> radar, USceneComponent* WeaponContainer, const FBoxSphereBounds& bodyBoundary);
    //virtual void BeginPlay() override;
    //virtual void PostInitializeComponents() override
    //{
    //    standardCos = Mathf.Cos(standard[0].LockAngle * Mathf.Deg2Rad);
    //    refreshTimer = -1.0f;
    //    gunRPM = 1.0f / (gunRPM / 60.0f);
    //}
    //
    //void LinkStandard(ReadOnlyCollection<Gauge> gauge)
    //{
    //    float value = standard[0].MaxCoolTime;
    //
    //    for (int i = 0; i < 2; i++)
    //    {
    //        gauge[i].LinkCoolTime(standard[i].GetCoolTime);
    //        gauge[i].GetMaxCoolTime(value);
    //    }
    //}
    //
    //void LinkSpecial(ReadOnlyCollection<Gauge> gauge)
    //{
    //    float value = specialSlot[special].MissileSlot[0].MaxCoolTime;
    //    for (int i = 0; i < maxCount; i++)
    //    {
    //        gauge[i].LinkCoolTime(specialSlot[special].MissileSlot[i].GetCoolTime);
    //        gauge[i].GetMaxCoolTime(value);
    //    }
    //}

    //===========================================
    // Methods
    //===========================================
	virtual void Update(float deltaTime);

    void ChangeTarget(void);
    void AddTarget(class AVehicleBase* target);
    void RemoveTarget(class AVehicleBase* target);
	void ReplaceSelectedTarget(class AVehicleBase* target);
    void SetTarget(const TWeakObjectPtr<class AVehicleBase>& target);

    inline bool ChangeMissile(void)
    {
        if (special == -1)
            return false;
    
        bSpecialSelected = !bSpecialSelected;
        return bSpecialSelected;
    }

    bool ShootMissile(TArray<int>& output);
    bool ShootMissile(void);
    bool ShootGun(void);
    bool ShootGun(const FVector& shootPosition, const FRotator& rotation);

    inline bool Lockable(const class AVehicleBase* vehicle) const { return Lockable(vehicle, GetMissileAimLayer()); }
    inline bool Lockable(const class AVehicleBase* vehicle, const FWeaponMask& mask) const;
    bool CalculateBombLandingPosition(FVector& OutLandingPosition);
    static bool CalculateBombLandingPosition(UWorld* World, const FVector& StartPosition, const FVector& InitialVelocity, FVector& OutLandingPosition, float& landingTime);

private:
    void RefreshTarget();

public:
    //===========================================
    // Variable & GetSet Methods
    //===========================================
    //DECLARE_MULTICAST_DELEGATE_TwoParams(FSelectTarget, class AVehicleBase*, bool active);
    //FSelectTarget SetSelectTargetUI;
    float SetCalculatedBombTime(float value) { return CalculatedBombTime; }
    

    TWeakObjectPtr<class AMissile> GetWeapon(int specialWeaponIndex);
    TWeakObjectPtr<class AMissile> GetMissile(bool isSpecial);
    void GetCurrentTargets(TArray<FTargetInfo>*& pointer) { pointer = &currentTargets; }
    const TArray<FTargetInfo>& GetCurrentTargets() const { return currentTargets; }

    void GetSelectSpecial(const bool*& pointer) const { pointer = &bSpecialSelected; }
    bool GetSelectSpecial() const { return bSpecialSelected; }
    int GetTeam() const { return Team; }
    int GetSpecial() const { return special; }
    int GetWeaponCount(bool isSpecialWeapon = true) const { if (isSpecialWeapon && special != -1) return MaxCount; return 2; }
    int GetMultiShoot() const { return multiShoot; }
    float GetCalculatedBombTime() const { return CalculatedBombTime; }

    inline const FWeaponMask& GetMissileAimLayer() const; // 지금 활성화된 무장의 레이어 반환
    inline const FWeaponMask& GetMissileAimLayer(bool isSpecial) const;// 무장(bool값으로 특수무장인지 일반무장인지)의 레이어 반환
    inline EWeaponID GetWeaponID(bool special = true) const;

protected:
    bool bSpecialSelected{ false }, bRefreshed{ false };
    int Team{ 0 }, special{ -1 }, MaxCount{ 1 }, multiShoot{ 1 };

    float standardCos{ 0.0f }, specialCos{ 0.0f };
    float refreshTime{ 0.0f }, refreshTimer{ 0.0f }, bulletTime{ 0.0f };
    float maxDegree{ FMath::Cos(FMath::DegreesToRadians(45.0f)) };
    float CalculatedBombTime{ 2.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Veriables", meta = (AllowPrivateAccess = "true"))
    float GunRPM = 350.0f; // 위치 변경해야되는데

    FVector GunPosition;

    TArray<TWeakObjectPtr<class AStandardMissile>> standard;
    TArray<FSpecialMissileSlot> specialSlots;

    TSortedMap<float, TWeakObjectPtr<class AVehicleBase>> Targets;
    TArray<FTargetInfo> currentTargets;

    TWeakObjectPtr<class URadar> Radar{ nullptr };
    FDelegateHandle AddHandle, RemoveHandle;
    TObjectPtr<class USoundBase> ShootSound, SpecialShootSound;
};
