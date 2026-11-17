// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundAIState.h"
#include "GroundVehicle.h"
#include "Radar.h"
#include "WeaponInterface.h"

GroundAIState::GroundAIState()
{
}

GroundAIState::~GroundAIState()
{
}

void GroundAIState::SetDefaultData(DefaultDataBlock* data)
{
    Release();

    AircraftDefaultData* detailData = static_cast<AircraftDefaultData*>(data);
    SquadData = detailData->SquadData;
    Owner = detailData->Owner;
    if (Owner == nullptr)
        return;

    Radar = Owner->GetRadar();
}







bool HoldState::Update()
{
    if (SearchInRadar())
    {
        NextState = State::Combat;
        TSharedPtr<AttackState::AttackStateData> data = MakeShared<AttackState::AttackStateData>();
        data->SelectedTarget = SelectedTarget;
        DataToSend = data;

        UE_LOG(LogTemp, Error, TEXT("[HoldState::Update] Searched %s -> %s"), *Owner->GetName(), *SelectedTarget->GetName());
        return true;
    }

    return false;
}

bool HoldState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    if (!GroundAIState::Start())
        return false;

    return true;
}

bool HoldState::SearchInRadar()
{
    const TSet<TWeakObjectPtr<class AVehicleBase>>& targets = Radar->GetTargets();

    if (targets.IsEmpty())
        return false;

    SelectedTarget.Reset();
    double minDistance = TNumericLimits<double>::Max();
    for (const TWeakObjectPtr<AVehicleBase>& item : targets)
    {
        if (!item.IsValid())
            continue;

        double distance = (item->GetActorLocation() - Owner->GetActorLocation()).SizeSquared();
        if (minDistance > distance)
        {
            minDistance = distance;
            SelectedTarget = item;
        }
    }

    if (!SelectedTarget.IsValid())
        return false;

    return true;
}



bool AttackState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    if (!GroundAIState::Start(data))
        return false;

    AttackStateData* detailData = static_cast<AttackStateData*>(data.Get());

    if (!detailData->SelectedTarget.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[AttackState::Start] ChangeToHold : NoTarget"));
        ChangeToHold();
        return false;
    }

    Targets->Reset();
    Targets->Emplace(detailData->SelectedTarget);
    Target = &(*Targets)[0];

    //LOG(TEXT("Combat %s->%s"), *Owner->GetName(), *Target->GetName());
    return true;
}

void AttackState::SetDefaultData(DefaultDataBlock* data)
{
    GroundAIState::SetDefaultData(data);

    FCS = Owner->GetFireControlSystem();
    UWeaponInterface* fcs = Cast<UWeaponInterface>(FCS);

    if (!IsValid(fcs))
    {
        Release();
        return;
    }

    //GunRange = GunRange * GunRange;
    FCS->GetCurrentTargets(Targets);

    TargetYaw = &fcs->GetTargetYaw();
    TargetPitch = &fcs->GetTargetPitch();
    TurretYaw = &fcs->GetTurretYaw();
    TurretPitch = &fcs->GetTurretPitch();
    
    MaxRange = Owner->GetRadar()->GetRaderDistance();
    MaxRange = MaxRange * MaxRange;

    if (fcs->CanShootGun())
        HandleWeapon.AddRaw(this, &AttackState::ShootGun);
    
    if (!fcs->IsMissileEmpty())
        HandleWeapon.AddRaw(this, &AttackState::ShootMissile);
}
    
void AttackState::Release(void)
{
    GroundAIState::Release();
    HandleWeapon.Clear();
    FCS.Reset();
}

bool AttackState::Update()
{
    static const double lockAngle = FMath::Cos(FMath::DegreesToRadians(2.5));
    if (Targets->IsEmpty() || !IsAlive(Target->Target))
    {
        NextState = State::Search;
        //LOG(TEXT("[CombatState::Update] %s !Target.IsValid"), *Owner->GetName());
        return true;
    }

    FVector directionVector = Target->Target->GetActorLocation() - Owner->GetActorLocation();
    FRotator targetRotation = directionVector.GetSafeNormal().Rotation();
    SquaredLength = directionVector.SquaredLength();

    if (SquaredLength >= MaxRange)
    {
        UE_LOG(LogTemp, Error, TEXT("[AttackState::Update] ChangeToHold %3f / %3f"), SquaredLength, MaxRange);
        ChangeToHold();
        return true;
    }

    HandleWeapon.Broadcast();

    return false;
}


void AttackState::ShootMissile()
{
    if (AttackTimer <= 0.0f)
    {
        if (Owner->GetFireControlSystem()->ShootMissile())
            AttackTimer = AttackFrequency;
    }
    else
        AttackTimer -= Owner->GetWorld()->GetDeltaSeconds();
}

void AttackState::ShootGun()
{
    //UE_LOG(LogTemp, Error, TEXT("[AttackState::ShootGun] %s / 1"), *Owner->GetName());
    //if (SquaredLength > GunRange)
    //    return;
    if (FMath::Abs(*TargetYaw) > 2.5f)
        return;
    if (FMath::Abs(*TargetPitch) > 2.5f)
        return;

    
    UE_LOG(LogTemp, Error, TEXT("[AttackState::ShootGun] %s / ShootGun"), *Owner->GetName());
    Owner->GetFireControlSystem()->ShootGun(FVector(50.0,0.0,0.0), FRotator(*TurretPitch, *TurretYaw, 0.0f));
}


void AttackState::ChangeToHold()
{
    //TSharedPtr<HoldState::ChaseStateDefaultData> newData = MakeShared<HoldState::ChaseStateDefaultData>();
    DataToSend = nullptr;
    NextState = State::Chase;
    Targets->Reset();
}


