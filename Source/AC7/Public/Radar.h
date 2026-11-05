// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Defined.h"
#include "Radar.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AC7_API URadar : public UActorComponent
{
	GENERATED_BODY()
public:
    //===========================================
    // Initializer/Destructor
    //===========================================
	URadar();
	virtual void BeginPlay() override;
	void SystemIntegration(TWeakObjectPtr<class USphereComponent> collider, int team, float raderDistance);
    void BindUI(class UIFFHandler* handler);
    //===========================================
    // Methods
    //===========================================
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    //UFUNCTION()
    //void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    void DeployFlare();
    void StartDeployFlare();
    void Trace(const TWeakObjectPtr<class AMissile>& missile) { Tracing.Add(missile); }
    void UnTrace(class AMissile* missile) { Tracing.Remove(missile); }
    void Remove(TArray<class AVehicleBase*>& targets);
    void CallWarning(double squareDistance);

    //public delegate void RaderEvent(Vehicle Target);
    //public delegate void WarningUIEvent(bool InDistance);
    //public event RaderEvent enterEvent;
    //public event RaderEvent exitEvent;
    //public event WarningUIEvent uiWarningEvent;
    
    
    //===========================================
    // Variable & GetSet Methods
    //===========================================
    DECLARE_MULTICAST_DELEGATE_OneParam(FVehicleFunction, class AVehicleBase*);
    FVehicleFunction AddTarget;
    FVehicleFunction RemoveTarget;

    void SetTeam(int value) { Team = value; }
    //void SetFlareCoolTime(float value) { flareCoolTime = value; }
    void SetAutoFlare() { AutoFlare = true; }
    void ResetWarning() { bAlart = bWarning = false; }

    bool Alart(void) { return bAlart; }
    bool Warning(void) { return bWarning; }
    float GetRaderDistance(void) { return RadarRange; }
    const TSet<TWeakObjectPtr<class AVehicleBase>>& GetTargets() const { return InDistance; } // Get Enemy Vehicle in Radar range
    const TArray<TWeakObjectPtr<class AVehicleBase>>& GetAllInRange() const { return InRange; } // Get All Vehicle in Radar range

private:
    FTimerHandle TimerHandle;

    bool bWarning{ false }, bAlart{ false };
    bool AutoFlare = false;
    int Team, layerMask, targetLayer;

    TSet<TWeakObjectPtr<class AMissile>> Tracing;
    TSet<TWeakObjectPtr<class AVehicleBase>> InDistance;
    TArray<TWeakObjectPtr<class AVehicleBase>> InRange;

    int FlareDeployCount{ 15 }, FlareCount{ 2 }, DeployAmount{ 0 };
    float FlareCoolTime{ 50.0f }, FlareRPM{ 550.0f };

    float RadarRange{ ConvertFromDistance(10000.0) };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<class USphereComponent> Collider;
    
    TArray<TWeakObjectPtr<USceneComponent>> flarePod;
    TWeakObjectPtr<class UAudioComponent> Audio{ nullptr };
    TObjectPtr<FControl> Control{ nullptr };

    TObjectPtr<USoundBase> FlareFire{ nullptr };
};
