// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Defined.h"
#include "Missile.generated.h"

UCLASS(Abstract)
class AC7_API AMissile : public AActor
{
	GENERATED_BODY()
public:
	AMissile();
    virtual void PostInitializeComponents(void) override;
    virtual void BeginPlay(void) override;

    virtual bool Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked, AMissile*& output); // return false at cooltime
    virtual bool Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked); // return false at cooltime
    virtual void Initialize(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked);
    virtual int Shoot(TWeakObjectPtr<class AVehicleBase> owner, FTargetInfo* locked);

    void ForceChangeTarget(TWeakObjectPtr<AActor> nextTarget);
    virtual void Lock(FTargetInfo& targetData, float DeltaTime, const FVector& forward, const FVector& position);
protected:
    //virtual TWeakObjectPtr<AMissile> CreateProjectile(TWeakObjectPtr<class AVehicleBase> targets) PURE_VIRTUAL(AMissile::CreateProjectile, return nullptr;);
    virtual TWeakObjectPtr<AMissile> CreateProjectile(TWeakObjectPtr<class AVehicleBase> targets);
    virtual TWeakObjectPtr<AMissile> CreateProjectile(void);
    virtual void Release();

    void CoolTimeEnd();
    void RetriveObject();
    void ResetMissileBind();

public:
    float GetLockAngle(void) const { return lockAngle; }
    float GetLockAngleCos(void) const { return AngleCos; }
    float GetMaxRange(void) const { return MaxRange; }
    int GetTargetCount(void) const { return targetCount; }
    int GetMultiShoot(void) const { return multiShoot; }
    int GetDamage(void) const { return damage; }
    const FWeaponMask& GetAimLayer() const { return AimLayer; }
    int GetAimMask() const { return AimLayer.GetMask(); }
    float GetLockSpeed() const { return lockSpeed; }
    float GetCoolTime() const { return Time; }
    float GetMaxCoolTime() const { return coolTime; }
    bool IsCoolTime() const { return bIsCoolTime; }
    EWeaponID GetWeaponID() const { return MissileID; }
   // public GaugeUI.GaugeUIType NeededUIType = > neededUI;
protected:
    bool bIsCoolTime{ false }, bIsProjectile{ false };
    float Time{ 0.0f }, DistanceSquare{ 0.0f }, AngleCos{ 0.0f }, FlyDistance{ TNumericLimits<float>::Max() };
    double ImpactDistance{ 100.0 }; // squared at runtime

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    EWeaponID MissileID{ EWeaponID::NULLID };
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    int damage = 70;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    int targetCount = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    float coolTime = 30.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    float MaxRange = 1000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    float velocity = 30.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    float rotationSpeed = 3.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    float lockAngle = 45.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    float lockSpeed = 1.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    int multiShoot = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SPEC", meta = (AllowPrivateAccess = "true"))
    FWeaponMask AimLayer;

    //GaugeUI.GaugeUIType neededUI;

    FTimerHandle CooldownTimerHandle;
    TWeakObjectPtr<class URadar> TargetRader{ nullptr };
    TWeakObjectPtr<AActor> Target{ nullptr };
	TWeakObjectPtr<class UStaticMeshComponent> BodyMesh{ nullptr };

    UPROPERTY(EditAnywhere)
    class UNiagaraComponent* Trail{ nullptr };

    DECLARE_DELEGATE_OneParam(FGiveRadarWarn, double)
    FGiveRadarWarn GiveRadarWarn;
};
