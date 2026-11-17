#include "AIAircraftState.h"
#include "EngineUtils.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"

#include "Aircraft.h"
#include "Radar.h"
#include "FireControlSystem.h"
#include "GroundAIState.h"
#include "TeamManagementSystem.h"
#include "FlightMovement.h"
#include "MovementPathAsset.h"

//#define DebugMessageEnable
#ifdef DebugMessageEnable
int32 DebugMessageIndex = 0;
#define BoolToString(value) (value ? TEXT("true") : TEXT("false"))
#define ScreenDebugMessage(message, ...) GEngine->AddOnScreenDebugMessage(DebugMessageIndex++, 0.f, FColor::Red, FString::Printf(message, ##__VA_ARGS__))

#define Log(index, message, ...) GEngine->AddOnScreenDebugMessage(index, 0.f, FColor::Red, FString::Printf(message, ##__VA_ARGS__))

#define LOG(Text, ...) UE_LOG(LogTemp, Log, Text, ##__VA_ARGS__)
#define Warn(Text, ...) UE_LOG(LogTemp, Warning, Text, ##__VA_ARGS__)
#define Err(Text, ...) UE_LOG(LogTemp, Error, Text, ##__VA_ARGS__)

#define ResetDebugMessageCount DebugMessageIndex = 0

#else
#define ScreenDebugMessage(message, ...) 
#define BoolToString(value)
#define ResetDebugMessageCount
#define LOG(Text, ...) UE_LOG(LogTemp, Log, Text, ##__VA_ARGS__)
#define Warn(Text, ...) UE_LOG(LogTemp, Warning, Text, ##__VA_ARGS__)
#define Err(Text, ...) UE_LOG(LogTemp, Error, Text, ##__VA_ARGS__)
#define Log(index, message, ...)
#endif


AIAircraftState::AIAircraftState()
{
}

AIAircraftState::~AIAircraftState()
{
}

bool AIAircraftState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    NextState = State::END;

    if (!IsAlive(Owner))
        return false;

    if (!TeamSystem.IsValid())
    {
		UWorld* world = Owner->GetWorld();
        if (world == nullptr)
        {
            UE_LOG(LogTemp, Error, TEXT("[AIAircraftState::Start] Owner->GetWorld() == nullptr"));
            return false;
        }

        TeamSystem = world->GetSubsystem<UTeamManagementSystem>();
        if (TeamSystem.IsValid())
            return false;
    }

    return true;
}

void AIAircraftState::SetDefaultData(DefaultDataBlock* data)
{
    Release();

    AircraftDefaultData* detailData = static_cast<AircraftDefaultData*>(data);
    Owner = detailData->Owner;
    if (Owner == nullptr)
        return;

    SquadData = &Owner->GetSquadData();
    Owner->GetControl(Control);
    Radar = Owner->GetRadar();

    if (Owner->GetWorld() != nullptr)
        TeamSystem = Owner->GetWorld()->GetSubsystem<UTeamManagementSystem>();
}

bool AIAircraftState::HorizontalRotation(const FVector& targetPosition, double* leftSquaredLength)
{
    FRotator aircraftRotation = Owner->GetActorRotation();
    FVector aircraftDirection = targetPosition - Owner->GetActorLocation();

    FRotator targetRotation = aircraftDirection.GetSafeNormal().Rotation();
    double squaredLength = FVector(aircraftDirection.X, aircraftDirection.Y, 0.0).SquaredLength();

    float yaw = FMath::UnwindDegrees(targetRotation.Yaw - aircraftRotation.Yaw);
    float yawABS = FMath::Abs(yaw);
    float pitch = FMath::UnwindDegrees(targetRotation.Pitch - aircraftRotation.Pitch);
    //float pitchABS = FMath::Abs(pitch);

    if (leftSquaredLength != nullptr) *leftSquaredLength = aircraftDirection.SquaredLength();

    ScreenDebugMessage(TEXT("[AIAircraftState::HorizontalRotation] Option :: yawABS %3f : Length %s %3f"), yawABS, (squaredLength <= CloseLengthSquare ? TEXT("Near") : TEXT("Far")), squaredLength);
    ScreenDebugMessage(TEXT("[AIAircraftState::HorizontalRotation] Yoke :: X : %3f, Y : %3f, Z : %3f"), Control->Yoke.X, Control->Yoke.Y, Control->Yoke.Z);
    ScreenDebugMessage(TEXT("[AIAircraftState::HorizontalRotation] Throttle : %3f, Velocity : %3f"), Control->Throttle, Control->Velocity);

    Control->isAirBrakeOn = false;

    if (yawABS < 15.0f)
        return StraightFollow(aircraftRotation, aircraftDirection, yaw, pitch);
    else if (yawABS < (squaredLength <= NearLengthSquare ? 35.0f : 22.5f))
        ShallowTurn(aircraftRotation, yaw, pitch);
    else if (yawABS >= 160.0f && squaredLength <= NearLengthSquare)
        RearFollow(aircraftRotation, targetRotation, pitch);
    else
        SteepTurn(aircraftRotation, yaw, -aircraftRotation.Pitch);

    ResetDebugMessageCount;
    return false;
}

void AIAircraftState::SteepTurn(FRotator& aircraftRotation, float yaw, float pitch)
{
    float reverse = yaw > 0.0f ? 1.0f : -1.0f;
    float angle = ClampRange((steepBankAngle * reverse - aircraftRotation.Roll) / 15.0f);
    Control->Yoke.X = angle;

    if (FMath::Abs(angle) <= 1.5f)
    {
        Control->Yoke.Y = ClampRange(pitch / 2.5f) * -reverse;
        Control->Yoke.Z = 1.0f;
    }
    else
    {
        Control->Yoke.Y = 0.0f;
        Control->Yoke.Z = ClampRange(-aircraftRotation.Pitch / 5.0f);
    }

    Control->Throttle = 0.6f;

    ScreenDebugMessage(TEXT("[AIAircraftState::SteepTurn] squaredLength : %3f"), 0.0);
    ScreenDebugMessage(TEXT("[AIAircraftState::SteepTurn] Yaw : %3f"), yaw);
}

void AIAircraftState::ShallowTurn(FRotator& aircraftRotation, float yaw, float pitch)
{
    Control->Yoke.X = ClampRange(((yaw > 0.0f ? bankAngle : -bankAngle) - aircraftRotation.Roll) / 10.0f);
    //Control->Yoke.Y = (FMath::Abs(Control->Yoke.X) < 0.1f) ? ClampRange(yaw) : 0.0f;
    Control->Yoke.Y = ClampRange(yaw);
    Control->Yoke.Z = ClampRange((FMath::Clamp(pitch, -5.0f, 5.0f) - aircraftRotation.Pitch) / 10.0f);

    Control->Throttle = 0.2f;
    if (Control->Velocity > 550.0f)
        Control->isAirBrakeOn = true;

    ScreenDebugMessage(TEXT("[AIAircraftState::ShallowTurn] rel pitch : %3f / pitch : %3f"), pitch, aircraftRotation.Pitch);
    ScreenDebugMessage(TEXT("[AIAircraftState::ShallowTurn] yaw : %3f"), yaw);
}

bool AIAircraftState::YawRotation(FRotator& aircraftRotation, float yaw, float pitch)
{
    Control->Yoke.X = ClampRange(-aircraftRotation.Roll / 10.0f);
    Control->Yoke.Y = ClampRange(yaw / 7.5f);
    Control->Yoke.Z = ClampRange(pitch / 15.0f);

    Control->isAirBrakeOn = false;
    Control->Throttle = 0.4f;
    return FMath::Abs(yaw) < 1.0f;
}

bool AIAircraftState::StraightFollow(FRotator& aircraftRotation, FVector& aircraftDirection, float yaw, float pitch)
{
    Control->Yoke.X = ClampRange(-aircraftRotation.Roll / 10.0f);
    Control->Yoke.Y = ClampRange(yaw / 7.5f);
    Control->Yoke.Z = ClampRange(pitch / 15.0f);

    float distance = aircraftDirection.SquaredLength();
    float weight = (0.95 + distance / CloseLengthSquare * 2.0f);
    Control->Throttle = 0.2f * weight;
    if (Control->Velocity > 33.0f * weight * 1.1f)
        Control->isAirBrakeOn = true;

    ScreenDebugMessage(TEXT("[AIAircraftState::StraightFollow] isAirBrakeOn : %s"), BoolToString(Control->isAirBrakeOn));
    ScreenDebugMessage(TEXT("[AIAircraftState::StraightFollow] distance : %3f / %3f"), distance, CloseLengthSquare);
    return distance <= CloseLengthSquare;
}

void AIAircraftState::RearFollow(FRotator& aircraftRotation, FRotator& targetRotation, float pitch)
{
    Control->Yoke.X = ClampRange(-aircraftRotation.Roll / 15.0f);
    Control->Yoke.Y = ClampRange(-FMath::UnwindDegrees(targetRotation.Yaw + (targetRotation.Yaw < 0.0f ? 180.0f : -180.0f)) / 7.5f);
    Control->Yoke.Z = ClampRange(pitch / 10.0f);

    Control->isAirBrakeOn = true;
    Control->Throttle = 0.2f;

    //ScreenDebugMessage(TEXT("[AIAircraftState::RearFollow] yaw : %3f"), FMath::UnwindDegrees(targetRotation.Yaw + 180.0f));
    ScreenDebugMessage(TEXT("[AIAircraftState::RearFollow] rel pitch : %3f / pitch : %3f"), pitch, aircraftRotation.Pitch);
    ScreenDebugMessage(TEXT("[AIAircraftState::RearFollow] tar pitch : %3f / pitch : %3f"), targetRotation.Pitch, aircraftRotation.Pitch);
}

bool AIAircraftState::Leveling()
{
    FRotator rotator = Owner->GetActorRotation();

    Control->Throttle = 0.2f;
    Control->isAirBrakeOn = false;
    Control->Yoke.X = ClampRange(-rotator.Roll / 10.0f);
    Control->Yoke.Y = 0.0f;
    Control->Yoke.Z = ClampRange(-rotator.Pitch / 10.0f);

    return FMath::Abs(rotator.Roll) < 1.0f && FMath::Abs(rotator.Pitch) < 1.0f;
}

bool AIAircraftState::SetAttitude(float targetPitch, float targetRoll, float targetYaw)
{
    FRotator rotator = Owner->GetActorRotation();

    Control->Throttle = 0.2f;
    Control->isAirBrakeOn = false;
    float angleDifferenceX = targetRoll - rotator.Roll;
    float angleDifferenceY = targetYaw - rotator.Yaw;
    float angleDifferenceZ = targetPitch - rotator.Pitch;

    Control->Yoke.X = ClampRange(angleDifferenceX / 10.0f);
    Control->Yoke.Y = ClampRange(angleDifferenceY / 10.0f);
    Control->Yoke.Z = ClampRange(angleDifferenceZ / 10.0f);

    return FMath::Abs(angleDifferenceX) < 1.0f || FMath::Abs(angleDifferenceY) < 1.0f || FMath::Abs(angleDifferenceZ) < 1.0f;
}

bool AIAircraftState::DefendAltitude(TSharedPtr<DataSetterBlock>& output)
{
    double altitude = Owner->GetActorLocation().Z;

    bool overMin = MinimumAltitude >= altitude;
    bool overMax = MaximumAltitude <= altitude;

    if (!overMin && !overMax)
        return false;

    TSharedPtr<OverridedDataBlock> defaultData = SetOverrideData();
    TSharedPtr<AltitudeSetterState::AltitudeSetterData> data = MakeShared<AltitudeSetterState::AltitudeSetterData>();
    NextState = AIState::State::Override_Altitude;
    data->PreviousData = defaultData->PreviousData;
    data->PriviousState = defaultData->PriviousState;
    data->TargetAltitude = overMin ? AscendAltitude : DescendAltitude;

    output = data;
    LOG(TEXT("[AIAircraftState::DefendAltitude] DefendAltitude"));
    return true;
}

bool AIAircraftState::NoticeWarning(TSharedPtr<DataSetterBlock>& output)
{
    if (Radar->Warning())
    {
        output = SetOverrideData();
        NextState = AIState::State::Override_Defend;
        LOG(TEXT("[AIAircraftState::NoticeWarning] EnterDefend %s"), *Owner->GetName());
        return true;
    }

    return false;
}

bool AIAircraftState::CogitateInFormation()
{
    if (!TeamSystem->LeaderCheck(*SquadData))
        return false;

    if (SquadData->IsLeader())
        return false;

    double squaredDistance = ((*SquadData->Leader)->GetActorLocation() - Owner->GetActorLocation()).SquaredLength();
    if (squaredDistance >= RangeLengthSquare * 2.0)
    {
        NextState = State::Follow;
        DataToSend.Reset();
        LOG(TEXT("[AIAircraftState::CogitateInFormation] EnterInFormation"));
        return true;
    }

    return false;
}

bool AIAircraftState::CogitateCombat()
{
    const TSet<TWeakObjectPtr<class AVehicleBase>>& list = Radar->GetTargets();

    if (list.IsEmpty())
        return false;

    TWeakObjectPtr<class AVehicleBase> closestTarget;
    double savedDistance = TNumericLimits<double>::Max();
    FVector location = Owner->GetActorLocation();

    for (const auto& item : list)
    {
        if (!IsAlive(item)) continue;
        double squaredDistance = (item->GetActorLocation() - location).SquaredLength();

        if (squaredDistance > RangeLengthSquare)
            continue;
        if (savedDistance <= squaredDistance)
            continue;

        closestTarget = item;
        savedDistance = squaredDistance;
    }

    if (!IsAlive(closestTarget))
        return false;

    TSharedPtr<CombatState::CombatStateDefaultData> newData = MakeShared<CombatState::CombatStateDefaultData>();
    newData->SelectedTarget = closestTarget;
    DataToSend = newData;
    NextState = State::Combat;
    return true;
}





bool InFomationState::Update()
{
    double leftSquaredLength;
    //float yaw;

    if (DefendAltitude(DataToSend))
        return true;

    if (CogitateCombat())
        return true;

    if (!TeamSystem->LeaderCheck(*SquadData))
    {
        NextState = State::Search;
        return true;
    }

    if (*SquadData->Leader == Owner)
    {
        NextState = State::Search;
        return true;
    }

    if (HorizontalRotation((*SquadData->Leader)->GetActorLocation() + SquadData->GetOffset(), &leftSquaredLength))
    {
        // match velocity but at least 10 over velocity
        float targetVelocity = FMath::Max(static_cast<AAircraft*>(SquadData->Leader->Get())->GetAirSpeed() * (0.95 + leftSquaredLength / CloseLengthSquare), 10.0f);
        Control->Throttle = Control->CalculateThrottle.Execute(targetVelocity);
        Control->isAirBrakeOn = targetVelocity < Control->Velocity * 1.05f;
    }

    return false;
}

bool InFomationState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    if (!AIAircraftState::Start()) return false;

    if (!TeamSystem->LeaderCheck(*SquadData))
    {
        NextState = State::Search;
        return false;
    }

    if (SquadData->IsLeader())
    {
        NextState = State::Search;
        return false;
    }

    return true;
}

TSharedPtr<InFomationState::OverridedDataBlock> InFomationState::SetOverrideData(void)
{
    TSharedPtr<OverridedDataBlock> detailData = MakeShared<OverridedDataBlock>();
    detailData->PriviousState = State::Follow;
    return detailData;
}





bool SearchState::Update()
{
    if (TaskHandle.IsCompleted())
    {
        if (IsAlive(SelectedTarget))
        {
            NextState = State::Chase;
            TSharedPtr<ChaseState::ChaseStateDefaultData> data = MakeShared<ChaseState::ChaseStateDefaultData>();
            data->SelectedTarget = SelectedTarget;
            DataToSend = data;

            return true;
        }

        if (SearchInRadar())
            return true;

        if (SquadData->IsLeader())
        {
            StartSearch();
        }
        else
        {
            NextState = State::Follow;
            return true;
        }
    }

    if (DefendAltitude(DataToSend))
        return true;
    if (NoticeWarning(DataToSend))
        return true;

    Leveling();
    return false;
}

bool SearchState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    if (!AIAircraftState::Start())
        return false;

    if (data != nullptr)
    {
        SearchData* detailData = static_cast<SearchData*>(data.Get());
        if (detailData->bSearching)
            return true;
    }

    if (SearchInRadar())
        return false;

    if (!SquadData->IsLeader())
    {
        NextState = State::Follow;
        return false;
    }

    StartSearch();
    return true;
}

TSharedPtr<SearchState::OverridedDataBlock> SearchState::SetOverrideData(void)
{
    TSharedPtr<OverridedDataBlock> detailData = MakeShared<OverridedDataBlock>();
    detailData->PriviousState = State::Search;

    TSharedPtr<SearchData> overridedData = MakeShared<SearchData>();
    overridedData->bSearching = TaskHandle.IsValid() && !TaskHandle.IsCompleted();
    detailData->PreviousData = overridedData;
    return detailData;
}

void SearchState::StartSearch()
{
    if (TaskHandle.IsValid() && !TaskHandle.IsCompleted())
        return;

    if (!Radar->GetTargets().IsEmpty())
        return;

    TArray<TPair<FVector, TWeakObjectPtr<AVehicleBase>>> targetArray;
    targetArray.Reserve(100);

    int teamIndex = Owner->GetTeam();
    for (TActorIterator<AVehicleBase> Iterator(Owner->GetWorld()); Iterator; ++Iterator)
    {
        if (teamIndex == (*Iterator)->GetTeam())
            continue;
        if (!IsAlive(*Iterator))
            continue;

        targetArray.Add({ (*Iterator)->GetActorLocation(), *Iterator });
    }

    SelectedTarget.Reset();

    TaskHandle = UE::Tasks::Launch(*FString(TEXT("AISearchTask::") + Owner->GetName()), [position = Owner->GetActorLocation(), result = &SelectedTarget, targetArray]() mutable
                                   {
                                       double minDistance = TNumericLimits<double>::Max();
                                       TWeakObjectPtr<AVehicleBase> selected;
                                       int length = targetArray.Num();

                                       for (int i = 0; i < length; i++)
                                       {
                                           double distance = (targetArray[i].Key - position).SizeSquared();
                                           if (minDistance > distance)
                                           {
                                               minDistance = distance;
                                               selected = targetArray[i].Value;
                                           }
                                       }

                                       *result = selected;
                                   });
    LOG(TEXT("[SearchState::StartSearch] Searching : %s"), *Owner->GetFName().ToString());
}

bool SearchState::SearchInRadar()
{
    const TSet<TWeakObjectPtr<class AVehicleBase>>& targets = Radar->GetTargets();

    if (targets.IsEmpty())
    {
        LOG(TEXT("[SearchState::SearchInRadar] No One In Radar"));
        return false;
    }

    SelectedTarget.Reset();
    double minDistance = TNumericLimits<double>::Max();
    for (const TWeakObjectPtr<AVehicleBase>& item : targets)
    {
        if (!IsAlive(item))
            continue;

        double distance = (item->GetActorLocation() - Owner->GetActorLocation()).SizeSquared();
        if (minDistance > distance)
        {
            minDistance = distance;
            SelectedTarget = item;
        }
    }

    if (!IsAlive(SelectedTarget))
        return false;

    TSharedPtr<CombatState::CombatStateDefaultData> newData = MakeShared<CombatState::CombatStateDefaultData>();
    newData->SelectedTarget = SelectedTarget;
    DataToSend = newData;
    NextState = State::Combat;
    LOG(TEXT("[SearchState::SearchInRadar] %s Searched : %s"), *Owner->GetFName().ToString(), *SelectedTarget->GetFName().ToString());
    return true;
}




bool ChaseState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    if (!AIAircraftState::Start(data))
        return false;

    if (!data.IsValid())
        return false;

    ChaseStateDefaultData* detailData = static_cast<ChaseStateDefaultData*>(data.Get());
    
    ChasingTarget = detailData->SelectedTarget;
    if (!IsAlive(detailData->SelectedTarget))
    {
        ChasingTarget.Reset();
        NextState = State::Search;
        return false;
    }

    double distance = (ChasingTarget->GetActorLocation() - Owner->GetActorLocation()).SquaredLength();
    if (distance <= RangeLengthSquare)
    {
        ChangeToCombat();
        return false;
    }

    return true;
}

TSharedPtr<ChaseState::OverridedDataBlock> ChaseState::SetOverrideData(void)
{
    TSharedPtr<OverridedDataBlock> detailData = MakeShared<OverridedDataBlock>();
    detailData->PriviousState = State::Chase;

    TSharedPtr<ChaseState::ChaseStateDefaultData> newData = MakeShared<ChaseState::ChaseStateDefaultData>();
    newData->SelectedTarget = ChasingTarget;
    detailData->PreviousData = newData;

    ChasingTarget.Reset();
    return detailData;
}

bool ChaseState::Update()
{
    if (DefendAltitude(DataToSend))
        return true;

    if (NoticeWarning(DataToSend))
        return true;

    if (CogitateInFormation())
        return true;

    if (!IsAlive(ChasingTarget))
    {
        NextState = State::Search;
        return true;
    }
    HorizontalRotation(ChasingTarget->GetActorLocation());

    double distance = (ChasingTarget->GetActorLocation() - Owner->GetActorLocation()).SquaredLength();
    if (distance < RangeLengthSquare)
    {
        ChangeToCombat();
        return false;
    }

    return false;
}

void ChaseState::ChangeToCombat()
{
    TSharedPtr<CombatState::CombatStateDefaultData> newData = MakeShared<CombatState::CombatStateDefaultData>();
    newData->SelectedTarget = ChasingTarget;
    DataToSend = newData;
    NextState = State::Combat;
}



bool CombatState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    if (!AIAircraftState::Start(data))
        return false;

    if (!data.IsValid())
        return false;

    CombatStateDefaultData* detailData = static_cast<CombatStateDefaultData*>(data.Get());
    Target = detailData->SelectedTarget;
    if (!IsAlive(Target))
    {
        Target.Reset();
        NextState = State::Search;
        LOG(TEXT("[CombatState::Start] Target is Not Alive State"));
        return false;
    }

    double distance = (Target->GetActorLocation() - Owner->GetActorLocation()).SquaredLength();
    if (distance > RangeLengthSquare)
    {
        LOG(TEXT("[CombatState::Start] Target out of range : %.3f > %.3f"), distance, RangeLengthSquare);
        ChangeToChase();
        return false;
    }
    Owner->SetTarget(Target);
    LOG(TEXT("Combat %s->%s"), *Owner->GetName(), *Target->GetName());
    return true;
}

void CombatState::SetDefaultData(DefaultDataBlock* data)
{
    AIAircraftState::SetDefaultData(data);

    if (Owner == nullptr)
        return;
}

TSharedPtr<CombatState::OverridedDataBlock> CombatState::SetOverrideData(void)
{
    TSharedPtr<OverridedDataBlock> detailData = MakeShared<OverridedDataBlock>();
    detailData->PriviousState = State::Combat;

    TSharedPtr<CombatState::CombatStateDefaultData> newData = MakeShared<CombatState::CombatStateDefaultData>();
    newData->SelectedTarget = Target;
    detailData->PreviousData = newData;
    Target.Reset();

    return detailData;
}

bool CombatState::Update()
{
    static const double lockAngle = FMath::Cos(FMath::DegreesToRadians(2.5));

    if (DefendAltitude(DataToSend))
        return true;
    if (NoticeWarning(DataToSend))
        return true;
    if (CogitateInFormation())
        return true;

    if (!IsAlive(Target))
    {
        NextState = State::Search;
        LOG(TEXT("[CombatState::Update] %s !Target.IsValid"), *Owner->GetName());
        return true;
    }

    FRotator aircraftRotation = Owner->GetActorRotation();
    FVector directionVector = Target->GetActorLocation() - Owner->GetActorLocation();
    FRotator targetRotation = directionVector.GetSafeNormal().Rotation();
    double squaredLength = FVector(directionVector.X, directionVector.Y, 0.0).SquaredLength();

    if (squaredLength >= RangeLengthSquare)
    {
        ChangeToChase();
        return true;
    }

    float yaw = FMath::UnwindDegrees(targetRotation.Yaw - aircraftRotation.Yaw);
    float yawABS = FMath::Abs(yaw);
    float pitch = FMath::UnwindDegrees(targetRotation.Pitch - aircraftRotation.Pitch);
    //float pitchABS = FMath::Abs(pitch);

    ScreenDebugMessage(TEXT("[CombatState::Update] Option :: yawABS %3f : Length %s %3f"), yawABS, (squaredLength <= CloseLengthSquare ? TEXT("Near") : TEXT("Far")), squaredLength);
    ScreenDebugMessage(TEXT("[CombatState::Update] Yoke :: X : %3f, Y : %3f, Z : %3f"), Control->Yoke.X, Control->Yoke.Y, Control->Yoke.Z);
    ScreenDebugMessage(TEXT("[CombatState::Update] Throttle : %3f, Velocity : %3f"), Control->Throttle, Control->Velocity);

    Control->isAirBrakeOn = false;

    if (yawABS < 15.0f)
        StraightFollow(aircraftRotation, directionVector, yaw, pitch);
    else if (yawABS < 20.0f)
        ShallowTurn(aircraftRotation, yaw, pitch);
    else
        SteepTurn(aircraftRotation, yaw, pitch - aircraftRotation.Pitch);

    Control->Throttle = FMath::Clamp(squaredLength / CloseLengthSquare, 0.2f, 1.0f);
    Control->isAirBrakeOn = false;

    ResetDebugMessageCount;

    if (AttackTimer <= 0.0f)
    {
        if (Owner->ShootMissile())
            AttackTimer = AttackFrequency;
    }
    else
        AttackTimer -= Owner->GetWorld()->GetDeltaSeconds();

    if (squaredLength <= CloseLengthSquare && FVector::DotProduct(Owner->GetActorForwardVector(), directionVector) > lockAngle)
        Owner->ShootGun();

    return false;
}

void CombatState::ChangeToChase()
{
    TSharedPtr<ChaseState::ChaseStateDefaultData> newData = MakeShared<ChaseState::ChaseStateDefaultData>();
    newData->SelectedTarget = Target;
    DataToSend = newData;
    NextState = State::Chase;
    Target.Reset();
}




bool AltitudeSetterState::Update()
{
    FRotator rotator = Owner->GetActorRotation();
    double altitude = TargetAltitude - Owner->GetActorLocation().Z;
    float targetAngle = static_cast<float>(FMath::Clamp(ClampRange(altitude / ConvertFromAltitude(200.0)), -1.0, 1.0)) * 45.0f;

    int i = 0;

    Log(i++, TEXT("[AltitudeSetterState::Update] Target : %3f, Current : %3f"), TargetAltitude, Owner->GetActorLocation().Z);
    Log(i++, TEXT("[AltitudeSetterState::Update] Yoke :: X : %3f, Y : %3f, Z : %3f"), Control->Yoke.X, Control->Yoke.Y, Control->Yoke.Z);
    Log(i++, TEXT("[AltitudeSetterState::Update] Throttle : %3f, Velocity : %3f"), Control->Throttle, Control->Velocity);

    Control->Throttle = 0.2f;
    Control->isAirBrakeOn = false;
    Control->Yoke.X = ClampRange(-rotator.Roll / 10.0f);
    Control->Yoke.Y = 0.0f;

    Control->Throttle = rotator.Pitch < 0 ? 0.2f : 0.8f;
    if (FMath::Abs(rotator.Roll) > 2.5f)
        Control->Yoke.Z = 0.0f;
    else
        Control->Yoke.Z = ClampRange(targetAngle - rotator.Pitch);

    Log(i++, TEXT("[AltitudeSetterState::Update] vtargetAngle : %3f, valtitude : %3f"), targetAngle, altitude);

    return FMath::Abs(altitude) < ConvertFromAltitude(10.0);
}

bool AltitudeSetterState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    if (!AIAircraftState::Start(data))
        return false;

    if (!data.IsValid())
        return false;

    AltitudeSetterData* detailData = static_cast<AltitudeSetterData*>(data.Get());
    TargetAltitude = detailData->TargetAltitude;
    DataToSend = detailData->PreviousData;
    NextState = detailData->PriviousState;
    return true;
}




bool DefendState::Update()
{
    float DeltaTime = Owner->GetWorld()->GetDeltaSeconds();

    if (Radar->Alart())
    {
        LOG(TEXT("[DefendState::Update] Alart - DeployFlare"));
        Radar->StartDeployFlare();
    }

    switch (Phase)
    {
    case 0:
        if (SetAttitude(InitialRotation.Pitch, InitialRotation.Roll, InitialRotation.Yaw))
        {
            Phase++;
            InitializeData();
            UE_LOG(LogTemp, Error, TEXT("[DefendState::Update] InitialRotation : %.3f, %.3f, %.3f"), InitialRotation.Pitch, InitialRotation.Roll, InitialRotation.Yaw);
        }
        break;
    case 1:
        SetAttitude(InitialRotation.Pitch, InitialRotation.Roll, InitialRotation.Yaw);

        if (!Owner->GetFlightMovement()->IsFollowingWayPoint())
        {
            UE_LOG(LogTemp, Error, TEXT("[DefendState::Update] OutOfTime"));
            Phase++;
            break;
        }
        else if (CollisionCheck == 0)
        {
            CollisionCheck = CheckWayCollision();

            if (CollisionCheck == -1)
                Phase = -1;
            else if (CollisionCheck == 1)
                Phase++;
            /*
            {
                FVector direction = WaypointData[0].Position - Owner->GetActorLocation();
                float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Owner->GetActorForwardVector().Dot(direction.GetSafeNormal()), -1.0f, 1.0f)));
                UE_LOG(LogTemp, Error, TEXT("[DefendState::Update] %s -> %s/ %s, distance = %.3f, angle = %.3f"), *Owner->GetActorLocation().ToString(), *WaypointData[0].Position.ToString(), *direction.ToString(), direction.Size(), Angle);
            }
            */

        }
        break;
    case 2:
        /*
    {
        FVector direction = WaypointData[0].Position - Owner->GetActorLocation();
        float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Owner->GetActorForwardVector().Dot(direction.GetSafeNormal()), -1.0f, 1.0f)));
        UE_LOG(LogTemp, Error, TEXT("[DefendState::Update] %s -> %s/ %s, distance = %.3f, angle = %.3f"), *Owner->GetActorLocation().ToString(), *WaypointData[0].Position.ToString(), *direction.ToString(), direction.Size(), Angle);
    }
        */
        Control->Yoke = FVector3f::ZeroVector;
    Owner->GetFlightMovement()->ClearWayPoint();
        Owner->GetFlightMovement()->SetWayPoint(WaypointData, false);
        WaypointData.Reset();
        Timer = DelayTime;
        Phase++;
        break;
    case 3:
        Control->Yoke = FVector3f::ZeroVector;
        if (!Owner->GetFlightMovement()->IsFollowingWayPoint())
            Phase++;
        break;
    default:
        Timer -= DeltaTime;

        Control->Yoke = FVector3f::ZeroVector;
        double altitude = Owner->GetActorLocation().Z;
		bool overMin = MinimumAltitude >= altitude;
        bool overMax = MaximumAltitude <= altitude;

        if (overMin || overMax)
        {
            TSharedPtr<AltitudeSetterState::AltitudeSetterData> data = MakeShared<AltitudeSetterState::AltitudeSetterData>();

            data->PreviousData = DataToSend;
            data->PriviousState = NextState;
			data->TargetAltitude = overMin ? AscendAltitude : DescendAltitude;

            NextState = AIState::State::Override_Altitude;
            DataToSend = data;
            Phase = -1;
            Timer = DelayTime;

            LOG(TEXT("[DefendState::Update] DefendAltitude"));
            return true;
        }

        if (Timer > 0.0f)
            break;

        if (Radar->Warning())
        {
            State state = NextState;
            TSharedPtr<DataSetterBlock> data = DataToSend;

			bool intiated = ManeuverState::Start();

            Timer = DelayTime;
            NextState = state;
            DataToSend = MoveTemp(data);

            LOG(TEXT("[DefendState::Start] DefendState Off : "), *Owner->GetName());
            return !intiated;
        }
        else return true;
    }

    return false;
}

bool DefendState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    if (!data.IsValid())
    {
        LOG(TEXT("[DefendState::Start] Override Failed"));
        return false;
    }

    if (AviliableManeuver.IsEmpty())
    {
        AviliableManeuver.Add(TEXT("VerticalTurn"));
        AviliableManeuver.Add(TEXT("HorizontalTurn"));
    }

    OverridedDataBlock* detailData = static_cast<OverridedDataBlock*>(data.Get());
	bool initialized = ManeuverState::Start();
    DataToSend = detailData->PreviousData;
    NextState = detailData->PriviousState;

    if (!initialized)
        return false;

    Timer = DelayTime;
    //Initiate();
    LOG(TEXT("[DefendState::Start] DefendState On : "), *Owner->GetName());
    return true;
}

void DefendState::Initiate() // deprecated
{
    double altitudeDifference = AltitudeMiddleValue - Owner->GetActorLocation().Z;
    Random = FMath::RandRange(-1.0, 1.0);

    TargetRotation = Owner->GetActorRotation();
    //TargetRotation.Pitch = 0.0;
    TargetRotation.Pitch = -FMath::Clamp(altitudeDifference / MinimumAltitude, -1.0, 1.0) * 2.5;
    TargetRotation.Yaw = FMath::UnwindDegrees(TargetRotation.Yaw + 180.0 + Random * 5.0);
    TargetRotation.Roll = 0.0;

    Timer = DelayTime;
    ReturnFlag = true;
}

TSharedPtr<DefendState::OverridedDataBlock> DefendState::SetOverrideData(void)
{
    TSharedPtr<OverridedDataBlock> detailData = MakeShared<OverridedDataBlock>();
    detailData->PriviousState = NextState;
	detailData->PreviousData = DataToSend;
    Phase = -1;
    Timer = DelayTime;
    return detailData;
}

void ManeuverState::Release(void)
{
    if (Handle.IsValid() && Handle.IsAwaitable())
        Handle.BusyWait();
}

bool ManeuverState::Update()
{
    float DeltaTime = Owner->GetWorld()->GetDeltaSeconds();
    switch (Phase)
    {
    case 0:
    {
        if (SetAttitude(InitialRotation.Pitch, InitialRotation.Roll, InitialRotation.Yaw))
        {
            Phase++;
            InitializeData();
            UE_LOG(LogTemp, Error, TEXT("[ManeuverState::Update] InitialRotation : %.3f, %.3f, %.3f"), InitialRotation.Pitch, InitialRotation.Roll, InitialRotation.Yaw);
        }
        break;
    }
    case 1:
        SetAttitude(InitialRotation.Pitch, InitialRotation.Roll, InitialRotation.Yaw);

        Timer -= DeltaTime;
        if (Timer <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("[ManeuverState::Update] OutOfTime"));
            Phase++;
            break;
        }

        if (CollisionCheck == 0)
        {
            CollisionCheck = CheckWayCollision();

            if (CollisionCheck == -1)
                Phase = -1;
            else if (CollisionCheck == 1)
                Phase++;
        }
        break;
    case 2:
        Control->Yoke = FVector3f::ZeroVector;
        Owner->GetFlightMovement()->ClearWayPoint();
        Owner->GetFlightMovement()->SetWayPoint(WaypointData, false);
        WaypointData.Reset();
        Phase++;
        break;
    case 3:
        Control->Yoke = FVector3f::ZeroVector;
        if (!Owner->GetFlightMovement()->IsFollowingWayPoint())
            Phase++;
        break;
    default:
        Control->Yoke = FVector3f::ZeroVector;
        if (CogitateCombat())
            return true;
        if (CogitateInFormation())
            return true;

        NextState = State::Search;
        return true;
    }

    return false;
}

bool ManeuverState::Start(const TSharedPtr<DataSetterBlock>& data)
{
    if (!AIAircraftState::Start(data))
        return false;

    TArray<FWaypointData> waypoint;
    UWorld* world = Owner->GetWorld();
    FString manuverName = TEXT("VerticalTurn"); // default

    if (data.IsValid())
    {
        ManeuverData* stateData = StaticCast<ManeuverData*>(data.Get());

        if (stateData != nullptr)
            manuverName = stateData->ManeuverName;
    }
    else if(!AviliableManeuver.IsEmpty())
        manuverName = AviliableManeuver[FMath::RandRange(1, AviliableManeuver.Num()) - 1];

    UMovementPathAsset::Load(*world, manuverName, WaypointData); // 디버깅을 위한 트래킹 이정표 -> f12

    if (WaypointData.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[ManeuverState::Start] Wrong Data"));
        return false;
    }

    CollisionCheck = Phase = 0;
    FRotator currentRotator = Owner->GetActorRotation();
    currentRotator.Roll = currentRotator.Pitch = 0.0f;
    InitialRotation = FRotator(WaypointData[0].Rotation * currentRotator.Quaternion());
    Control->Throttle = 0.2f;

    return true;
}

int ManeuverState::CheckWayCollision(void)
{
    int length = HandleList.Num();
    FOverlapDatum overlapData;
    UWorld* world = Owner->GetWorld();

    for (int i = length - 1; i >= 0; i--)
    {
        auto& handle = HandleList[i];
        if (world->QueryOverlapData(handle, overlapData))
        {
            if (overlapData.OutOverlaps.IsEmpty())
                HandleList.Pop();
            else
                //return -1;
            {
                Err(TEXT("[ManeuverState::CheckWayCollision] OutOverlaps -> Check skipped due to test"));
                for (auto& item : overlapData.OutOverlaps)
                    Err(TEXT("[ManeuverState::CheckWayCollision] OutOverlaps : %s"), *item.GetActor()->GetActorLabel());

                HandleList.Pop();
            }
        }
        else
            return 0;
    }

    if (HandleList.IsEmpty())
        return 1;

    return -1;
}

float ManeuverState::GetRotationTime(const FQuat& StartQuat, const FQuat& TargetQuat, const FVector& AngularSpeed)
{
    auto GetAxisTime = [](float DeltaAngle, float Speed) -> float
        {
            if (FMath::IsNearlyZero(DeltaAngle))
                return 0.0f;

            if (FMath::IsNearlyZero(Speed))
                return TNumericLimits<float>::Max();

            return FMath::Abs(DeltaAngle) / FMath::Abs(Speed);
        };

    FRotator StartRot = StartQuat.Rotator();
    FRotator TargetRot = TargetQuat.Rotator();

    float DeltaRoll = FMath::FindDeltaAngleDegrees(StartRot.Roll, TargetRot.Roll);
    float DeltaPitch = FMath::FindDeltaAngleDegrees(StartRot.Pitch, TargetRot.Pitch);
    float DeltaYaw = FMath::FindDeltaAngleDegrees(StartRot.Yaw, TargetRot.Yaw);

    float RollTime = GetAxisTime(DeltaRoll, AngularSpeed.X);
    float PitchTime = GetAxisTime(DeltaPitch, AngularSpeed.Y);
    float YawTime = GetAxisTime(DeltaYaw, AngularSpeed.Z);

    return FMath::Max3(RollTime, PitchTime, YawTime);
}

bool ManeuverState::InitializeData(void)
{
    if (WaypointData.IsEmpty())
        return false;

    CollisionCheck = -2;
    HandleList.SetNum(0, false);
    FRotator currentRotator = Owner->GetActorRotation();
    currentRotator.Pitch = currentRotator.Roll = 0.0f;
    FQuat yawQuaternion = currentRotator.Quaternion();
    
    InitialPosition = Owner->GetActorLocation() + Owner->GetActorForwardVector() * ConvertFromVelocity(Control->Velocity) * IdleTimer;
    InitialRotation = FRotator(yawQuaternion * WaypointData[0].Rotation);

    /*{
        FVector direction = InitialPosition - Owner->GetActorLocation();
        float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Owner->GetActorForwardVector().Dot(direction.GetSafeNormal()), -1.0f, 1.0f)));
        UE_LOG(LogTemp, Error, TEXT("[ManeuverState::InitializeData] %s -> %s/ %s, distance = %.3f, angle = %.3f"), *Owner->GetActorLocation().ToString(), *InitialPosition.ToString(), *direction.ToString(), direction.Size(), Angle);
    }*/

    Control->Throttle = 0.2f;
    Timer = IdleTimer;
    Control->Yoke = FVector3f::ZeroVector;
    Owner->GetFlightMovement()->InjectWayPoint(FWaypointData(InitialPosition, InitialRotation.Quaternion(), 0.0));

    FString taskName = TEXT("[ManeuverState::InitializeData]") + Owner->GetName();
    double LaunchTime = FPlatformTime::Seconds();
    TWeakObjectPtr<AVehicleBase> subject = Owner;
    Handle = UE::Tasks::Launch(*taskName, [this, subject, waypointList = MoveTemp(WaypointData), yawQuaternion, LaunchTime]() mutable -> void
                               {
                                   double StartTime = FPlatformTime::Seconds();
                                   LOG(TEXT("[ManeuverState::InitializeData] StartThread / Wait Time : %.3f ms / Num : %d"), (StartTime - LaunchTime) * 1000.0, waypointList.Num());

                                   for (auto& item : waypointList)
                                   {
                                       item.Position = yawQuaternion.RotateVector(item.Position) + InitialPosition;
                                       item.Rotation = yawQuaternion * item.Rotation;
                                   }

                                   AsyncTask(ENamedThreads::GameThread, [this, subject, waypoints = MoveTemp(waypointList)]() mutable -> void
                                             {
                                                 FCollisionQueryParams Params;
                                                 FCollisionResponseParams ResponseParams;
                                                 int jump = 1;
                                                 UWorld* world = Owner->GetWorld();
                                                 if (!IsAlive(Owner) || world == nullptr) { CollisionCheck = -1; return; }
                                                 LOG(TEXT("[ManeuverState::InitializeData] EndThread / Num : %d"), WaypointData.Num());

                                                 CollisionCheck = 0;
                                                 WaypointData = MoveTemp(waypoints);


                                                 if (WaypointData.Num() > 1)
                                                     jump = FMath::Max(1, FMath::FloorToInt(Radius / (WaypointData[0].Position - WaypointData[1].Position).Length()));

                                                 HandleList.Reserve(WaypointData.Num() / jump);
                                                 FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
                                                 ECollisionChannel channel = ECollisionChannel::ECC_WorldStatic;

                                                 uint32 count = 0;
                                                 for (int32 i = 0; i < WaypointData.Num(); i += jump)
                                                 {
                                                     auto trace = world->AsyncOverlapByChannel(WaypointData[i].Position, FQuat::Identity, channel, Shape, Params, ResponseParams, nullptr, count++);
                                                     HandleList.Add(trace);
                                                 }

                                                 /*
                                                 for (int32 i = 0; i + 1 < WorldPoints.Num(); i += 2)
                                                 {
                                                     const FVector& Start = WorldPoints[i];
                                                     const FVector& End = WorldPoints[i + 1];

                                                     World->AsyncSweepByChannel(EAsyncTraceType::Test, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius), Params, ResponseParams, nullptr, SegmentIndex);
                                                     World->AsyncLineTraceByChannel(EAsyncTraceType::Test, Start, End, ECC_Visibility, Params, ResponseParams, nullptr, static_cast<uint32>(i / 2));
                                                 }
                                                 */
                                             });

                                   LOG(TEXT("EndThread / Working Time : %.3f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.0);
                               });
    return true;
}
