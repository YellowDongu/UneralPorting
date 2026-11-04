#include "WaypointEditorData.h"

// 회전 보간 방식 스위치: true = Squad(각속도 연속), false = 이전 방식(구간별 Slerp)
static constexpr bool bUseSquadRotation = true;


FWaypointEditorData::FWaypointEditorData()
{
    OnChanged.AddRaw(this, &FWaypointEditorData::BuildSpline);
}

void FWaypointEditorData::SelectWaypoint(int32 Index)
{
    SelectedWaypointIndex = Waypoints.IsValidIndex(Index) ? Index : INDEX_NONE;
    OnChanged.Broadcast();
}

void FWaypointEditorData::AddWaypoint(const FVector& Position)
{
    Waypoints.Emplace(Position, FRotator::ZeroRotator);
    SelectedWaypointIndex = Waypoints.Num() - 1;

    OnChanged.Broadcast();
}

void FWaypointEditorData::RemoveSelectedWaypoint(void)
{
    if (!HasSelection())
        return;

    Waypoints.RemoveAt(SelectedWaypointIndex);
    SelectedWaypointIndex = INDEX_NONE;
    OnChanged.Broadcast();
}

void FWaypointEditorData::SetSelectedPosition(const FVector& Position)
{
    if (!HasSelection())
        return;

    Waypoints[SelectedWaypointIndex].Position = Position;
    OnChanged.Broadcast();
}

void FWaypointEditorData::SetSelectedRotation(const FRotator& Rotation)
{
    if (!HasSelection())
        return;

    Waypoints[SelectedWaypointIndex].Rotation = Rotation;
    OnChanged.Broadcast();
}

void FWaypointEditorData::OffsetAllPositions(const FVector& Delta)
{
    if (Waypoints.IsEmpty() || Delta.IsNearlyZero())
        return;

    for (FWaypoint& Waypoint : Waypoints)
        Waypoint.Position += Delta;

    OnChanged.Broadcast(); // 전부 수정한 뒤 한 번만 브로드캐스트해서 스플라인 재빌드를 1회로 제한
}

void FWaypointEditorData::ScaleAllPositions(const FVector& Scale)
{
    if (Waypoints.IsEmpty() || Scale.Equals(FVector::OneVector))
        return;

    // 기준점은 데이터 로컬 좌표계의 원점(0,0,0)이므로 성분별 곱만 하면 된다
    for (FWaypoint& Waypoint : Waypoints)
        Waypoint.Position *= Scale;

    OnChanged.Broadcast();
}

void FWaypointEditorData::RemoveWaypoint(int32 Index)
{
    if (!Waypoints.IsValidIndex(Index))
        return;

    Waypoints.RemoveAt(Index);

    if (Waypoints.IsEmpty())
        SelectedWaypointIndex = INDEX_NONE;
    else if (SelectedWaypointIndex == Index)
        SelectedWaypointIndex = INDEX_NONE;
    else if (SelectedWaypointIndex > Index)
        --SelectedWaypointIndex;

    OnChanged.Broadcast();
}

void FWaypointEditorData::MoveWaypoint(int32 FromIndex, int32 ToIndex)
{
    if (!Waypoints.IsValidIndex(FromIndex) || !Waypoints.IsValidIndex(ToIndex) || FromIndex == ToIndex)
        return;

    auto Position = Waypoints[FromIndex];

    Waypoints.RemoveAt(FromIndex);
    Waypoints.Insert(Position, ToIndex);

    if (SelectedWaypointIndex == FromIndex)
        SelectedWaypointIndex = ToIndex;
    else if (FromIndex < SelectedWaypointIndex && ToIndex >= SelectedWaypointIndex)
        --SelectedWaypointIndex;
    else if (FromIndex > SelectedWaypointIndex && ToIndex <= SelectedWaypointIndex)
        ++SelectedWaypointIndex;

    OnChanged.Broadcast();
}

void FWaypointEditorData::RebuildSpline(void)
{
    SplineSamples.Reset();
    int32 Count = Waypoints.Num();
    if (Count == 0)
        return;

    if (Count == 1)
    {
        SplineSamples.Add(Waypoints[0].Position);
        return;
    }

    int32 length = Count - 1;
    for (int32 i = 0; i < length; ++i)
    {
        const FVector& P0 = Waypoints[FMath::Max(i - 1, 0)].Position;
        const FVector& P1 = Waypoints[i].Position;
        const FVector& P2 = Waypoints[i + 1].Position;
        const FVector& P3 = Waypoints[FMath::Min(i + 2, length)].Position;

        for (int32 j = 0; j < SubdivisionsPerSegment; ++j)
            SplineSamples.Add(EvaluateCatmullRom(P0, P1, P2, P3, static_cast<float>(j) / SubdivisionsPerSegment));
    }

    SplineSamples.Add(Waypoints.Last().Position); // 마지막 웨이포인트까지 포함
}

FVector FWaypointEditorData::EvaluateCatmullRom(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T)
{
    float T2 = T * T;
    float T3 = T2 * T;

    return 0.5f * (2.0f * P1 + (-P0 + P2) * T + (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * T2 + (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * T3);
}

void FWaypointEditorData::AppendAdaptiveSamples(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T0, float T1, const FVector& Start, const FVector& End, float MaxChordLength, float FlatnessTolerance, int32 Depth, TArray<FVector>& OutSamples)
{
    float MidT = (T0 + T1) * 0.5f;
    FVector Mid = EvaluateCatmullRom(P0, P1, P2, P3, MidT);
    FVector Chord = End - Start;
    float ChordLengthSquared = Chord.SizeSquared(); // 중간점과 시작-끝 직선 사이의 거리 계산
    float Alpha = 0.0f;

    if (ChordLengthSquared > KINDA_SMALL_NUMBER)
    {
        Alpha = FVector::DotProduct(Mid - Start, Chord) / ChordLengthSquared;
        Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
    }

    FVector ClosestPoint = Start + Chord * Alpha;
    float Deviation = FVector::Distance(Mid, ClosestPoint);
    float ChordLength = FVector::Distance(Start, End);
    bool bNeedsSubdivision = ChordLength > MaxChordLength || Deviation > FlatnessTolerance;

    if (bNeedsSubdivision && Depth < 12) // 최대 깊이에 도달하면 더 이상 분할하지 않음 -> 스택 폭주 제어
    {
        AppendAdaptiveSamples(P0, P1, P2, P3, T0, MidT, Start, Mid, MaxChordLength, FlatnessTolerance, Depth + 1, OutSamples);
        AppendAdaptiveSamples(P0, P1, P2, P3, MidT, T1, Mid, End, MaxChordLength, FlatnessTolerance, Depth + 1, OutSamples);
    }
    else
        OutSamples.Add(End);
}
/*
void FWaypointEditorData::BuildDistanceSamples()
{
    DistanceSamples.Reset();
    DistanceSampleDistances.Reset();
    TotalSplineLength = 0.0f;

    const int32 Count = Waypoints.Num();

    if (Count == 0)
        return;

    if (DistanceSampleSpacing <= KINDA_SMALL_NUMBER)
        return;

    if (Count == 1)
    {
        DistanceSamples.Add(Waypoints[0]);
        DistanceSampleDistances.Add(0.0f);
        return;
    }

    TArray<FVector> CurveSamples; // 곡선 길이를 근사하기 위한 내부 샘플
    const int32 CurveSubdivisions = 100;

    for (int32 i = 0; i < Count - 1; ++i)
    {
        const FVector& P0 = Waypoints[FMath::Max(i - 1, 0)];
        const FVector& P1 = Waypoints[i];
        const FVector& P2 = Waypoints[i + 1];
        const FVector& P3 = Waypoints[FMath::Min(i + 2, Count - 1)];

        for (int32 j = 0; j < CurveSubdivisions; ++j)
        {
            float T = static_cast<float>(j) / CurveSubdivisions;
            CurveSamples.Add(EvaluateCatmullRom(P0, P1, P2, P3, T));

            continue;
            float T2 = T * T;
            float T3 = T2 * T;

            FVector Position = 0.5f * (2.0f * P1 + (-P0 + P2) * T + (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * T2 + (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * T3);

            CurveSamples.Add(Position);
        }
    }

    CurveSamples.Add(Waypoints.Last());

    // 내부 샘플을 이용해 누적 거리 계산
    TArray<float> CumulativeDistances;
    CumulativeDistances.SetNum(CurveSamples.Num());
    CumulativeDistances[0] = 0.0f;

    for (int32 i = 1; i < CurveSamples.Num(); ++i)
    {
        CumulativeDistances[i] = CumulativeDistances[i - 1] + FVector::Distance(CurveSamples[i - 1], CurveSamples[i]);
    }

    TotalSplineLength = CumulativeDistances.Last();

    DistanceSamples.Add(CurveSamples[0]);
    DistanceSampleDistances.Add(0.0f);

    if (TotalSplineLength <= KINDA_SMALL_NUMBER)
        return;

    int32 SegmentIndex = 0;

    for (float TargetDistance = DistanceSampleSpacing; TargetDistance < TotalSplineLength; TargetDistance += DistanceSampleSpacing)
    {
        while (SegmentIndex < CurveSamples.Num() - 2 && CumulativeDistances[SegmentIndex + 1] < TargetDistance)
            ++SegmentIndex;

        float StartDistance = CumulativeDistances[SegmentIndex];
        float EndDistance = CumulativeDistances[SegmentIndex + 1];
        float SegmentLength = EndDistance - StartDistance;

        if (SegmentLength <= KINDA_SMALL_NUMBER)
            continue;

        float Alpha = (TargetDistance - StartDistance) / SegmentLength;
        FVector Position = FMath::Lerp(CurveSamples[SegmentIndex], CurveSamples[SegmentIndex + 1], Alpha);

        DistanceSamples.Add(Position);
        DistanceSampleDistances.Add(TargetDistance);
    }

    DistanceSamples.Add(CurveSamples.Last());
    DistanceSampleDistances.Add(TotalSplineLength);
}
*/

void FWaypointEditorData::BuildSpline(void)
{
    DistanceSamples.Reset();
    DistanceSampleDistances.Reset();
    DistanceSampleRotations.Reset();                 // [추가]
    TotalSplineLength = 0.0f;
    
    int32 Count = Waypoints.Num();

    if (Count == 0 || DistanceSampleSpacing <= KINDA_SMALL_NUMBER)
        return;

    if (Count == 1)
    {
        DistanceSamples.Add(Waypoints[0].Position);
        DistanceSampleDistances.Add(0.0f);
        DistanceSampleRotations.Add(Waypoints[0].Rotation.Quaternion());   // [추가]
        return;
    }

    float MaxChordLength = DistanceSampleSpacing * 0.5f;
    float FlatnessTolerance = FMath::Max(0.1f, DistanceSampleSpacing * 0.01f);

    TArray<FVector> CurveSamples;
    TArray<int32> WaypointCurveIndex;                // [추가] k번 웨이포인트가 CurveSamples의 몇 번째인지
    CurveSamples.Add(Waypoints[0].Position);
    WaypointCurveIndex.Add(0);                       // [추가] 0번 웨이포인트

    for (int32 i = 0; i < Count - 1; ++i)
    {
        const FVector& P0 = Waypoints[FMath::Max(i - 1, 0)].Position;
        const FVector& P1 = Waypoints[i].Position;
        const FVector& P2 = Waypoints[i + 1].Position;
        const FVector& P3 = Waypoints[FMath::Min(i + 2, Count - 1)].Position;
        float ChordDistance = FVector::Distance(P1, P2);
        int32 InitialSubdivisions = FMath::Max(1, FMath::CeilToInt(ChordDistance / DistanceSampleSpacing));

        for (int32 j = 0; j < InitialSubdivisions; ++j)
        {
            float T0 = static_cast<float>(j) / InitialSubdivisions;
            float T1 = static_cast<float>(j + 1) / InitialSubdivisions;

            FVector Start = EvaluateCatmullRom(P0, P1, P2, P3, T0);
            FVector End = EvaluateCatmullRom(P0, P1, P2, P3, T1);

            AppendAdaptiveSamples(P0, P1, P2, P3, T0, T1, Start, End, MaxChordLength, FlatnessTolerance, 0, CurveSamples);
        }

        // [추가] 방금 마지막으로 들어간 샘플(T=1)이 i+1번 웨이포인트
        WaypointCurveIndex.Add(CurveSamples.Num() - 1);
    }

    // 누적 거리 (기존 그대로)
    TArray<float> CumulativeDistances;
    CumulativeDistances.SetNum(CurveSamples.Num());
    CumulativeDistances[0] = 0.0f;

    for (int32 i = 1; i < CurveSamples.Num(); ++i)
        CumulativeDistances[i] = CumulativeDistances[i - 1] + FVector::Distance(CurveSamples[i - 1], CurveSamples[i]);

    TotalSplineLength = CumulativeDistances.Last();

    // [추가] 각 웨이포인트의 시작점으로부터의 거리
    TArray<float> WaypointDistances;
    WaypointDistances.SetNum(Count);
    for (int32 k = 0; k < Count; ++k)
        WaypointDistances[k] = CumulativeDistances[WaypointCurveIndex[k]];

    DistanceSamples.Add(CurveSamples[0]);
    DistanceSampleDistances.Add(0.0f);
    DistanceSampleRotations.Add(Waypoints[0].Rotation.Quaternion());      // [추가]

    if (TotalSplineLength <= KINDA_SMALL_NUMBER)
        return;

    int32 SegmentIndex = 0;
    // 웨이포인트 회전용 Squad 탄젠트 (bUseSquadRotation == true일 때만 계산)
    TArray<FQuat> WaypointQuats, WaypointTangents;
    if constexpr (bUseSquadRotation)
    {
        WaypointQuats.SetNum(Count);
        WaypointTangents.SetNum(Count);

        for (int32 k = 0; k < Count; ++k)
        {
            WaypointQuats[k] = Waypoints[k].Rotation.Quaternion();
            if (k > 0 && (WaypointQuats[k] | WaypointQuats[k - 1]) < 0.0f)
                WaypointQuats[k] = -WaypointQuats[k]; // 인접 웨이포인트끼리 최단 경로가 되도록 부호 정렬
        }

        for (int32 k = 0; k < Count; ++k)
            FQuat::CalcTangents(WaypointQuats[FMath::Max(k - 1, 0)], WaypointQuats[k], WaypointQuats[FMath::Min(k + 1, Count - 1)], 0.0f, WaypointTangents[k]);
    }

    int32 WaypointSegment = 0;                                            // [추가]

    for (float TargetDistance = DistanceSampleSpacing; TargetDistance < TotalSplineLength; TargetDistance += DistanceSampleSpacing)
    {
        while (SegmentIndex < CurveSamples.Num() - 2 && CumulativeDistances[SegmentIndex + 1] < TargetDistance)
            ++SegmentIndex;

        float StartDistance = CumulativeDistances[SegmentIndex];
        float EndDistance = CumulativeDistances[SegmentIndex + 1];
        float SegmentLength = EndDistance - StartDistance;

        if (SegmentLength <= KINDA_SMALL_NUMBER)
            continue;

        float Alpha = (TargetDistance - StartDistance) / SegmentLength;
        FVector Position = FMath::Lerp(CurveSamples[SegmentIndex], CurveSamples[SegmentIndex + 1], Alpha);

        // [추가] TargetDistance가 어느 두 웨이포인트 사이인지 찾고, 그 안에서의 비율로 회전 보간
        while (WaypointSegment < Count - 2 && WaypointDistances[WaypointSegment + 1] < TargetDistance)
            ++WaypointSegment;

        float WStart = WaypointDistances[WaypointSegment];
        float WLength = WaypointDistances[WaypointSegment + 1] - WStart;
        float RotAlpha = WLength > KINDA_SMALL_NUMBER ? FMath::Clamp((TargetDistance - WStart) / WLength, 0.0f, 1.0f) : 0.0f;

        FQuat Rotation;
        if constexpr (bUseSquadRotation)
            Rotation = FQuat::Squad(WaypointQuats[WaypointSegment], WaypointTangents[WaypointSegment], WaypointQuats[WaypointSegment + 1], WaypointTangents[WaypointSegment + 1], RotAlpha);
        else
            Rotation = FQuat::Slerp(Waypoints[WaypointSegment].Rotation.Quaternion(), Waypoints[WaypointSegment + 1].Rotation.Quaternion(), RotAlpha); // 이전 방식
        // [\추가]

        DistanceSamples.Add(Position);
        DistanceSampleDistances.Add(TargetDistance);
        DistanceSampleRotations.Add(Rotation);                            // [추가]
    }

    DistanceSamples.Add(CurveSamples.Last());
    DistanceSampleDistances.Add(TotalSplineLength);
    DistanceSampleRotations.Add(bUseSquadRotation ? WaypointQuats.Last() : Waypoints.Last().Rotation.Quaternion()); // [추가]

    return;
    /*

    DistanceSamples.Reset();
    DistanceSampleDistances.Reset();
    TotalSplineLength = 0.0f;

    int32 Count = Waypoints.Num();

    if (Count == 0 || DistanceSampleSpacing <= KINDA_SMALL_NUMBER)
        return;

    if (Count == 1)
    {
        DistanceSamples.Add(Waypoints[0].Position);
        DistanceSampleDistances.Add(0.0f);
        return;
    }

    // 거리 기준으로 곡선 근사 정밀도를 설정
    float MaxChordLength = DistanceSampleSpacing * 0.5f;
    float FlatnessTolerance = FMath::Max(0.1f, DistanceSampleSpacing * 0.01f);

    TArray<FVector> CurveSamples;
    CurveSamples.Add(Waypoints[0].Position);

    for (int32 i = 0; i < Count - 1; ++i) // 직선거리를 기준으로 초기 분할
    {
        FVector& P0 = Waypoints[FMath::Max(i - 1, 0)].Position;
        FVector& P1 = Waypoints[i].Position;
        FVector& P2 = Waypoints[i + 1].Position;
        FVector& P3 = Waypoints[FMath::Min(i + 2, Count - 1)].Position;
        float ChordDistance = FVector::Distance(P1, P2);
        int32 InitialSubdivisions = FMath::Max(1, FMath::CeilToInt(ChordDistance / DistanceSampleSpacing));

        for (int32 j = 0; j < InitialSubdivisions; ++j)
        {
            float T0 = static_cast<float>(j) / InitialSubdivisions;
            float T1 = static_cast<float>(j + 1) / InitialSubdivisions;

            FVector Start = EvaluateCatmullRom(P0, P1, P2, P3, T0);
            FVector End = EvaluateCatmullRom(P0, P1, P2, P3, T1);

            AppendAdaptiveSamples(P0, P1, P2, P3, T0, T1, Start, End, MaxChordLength, FlatnessTolerance, 0, CurveSamples);
        }
    }

    // 곡선 근사 샘플의 누적 거리 계산
    TArray<float> CumulativeDistances;
    CumulativeDistances.SetNum(CurveSamples.Num());
    CumulativeDistances[0] = 0.0f;

    for (int32 i = 1; i < CurveSamples.Num(); ++i)
        CumulativeDistances[i] = CumulativeDistances[i - 1] + FVector::Distance(CurveSamples[i - 1], CurveSamples[i]);

    TotalSplineLength = CumulativeDistances.Last();
    DistanceSamples.Add(CurveSamples[0]);
    DistanceSampleDistances.Add(0.0f);

    if (TotalSplineLength <= KINDA_SMALL_NUMBER)
        return;

    int32 SegmentIndex = 0;
    for (float TargetDistance = DistanceSampleSpacing; TargetDistance < TotalSplineLength; TargetDistance += DistanceSampleSpacing) // 누적 거리를 기준으로 최종 위치 추출
    {
        while (SegmentIndex < CurveSamples.Num() - 2 && CumulativeDistances[SegmentIndex + 1] < TargetDistance)
            ++SegmentIndex;


        float StartDistance = CumulativeDistances[SegmentIndex];
        float EndDistance = CumulativeDistances[SegmentIndex + 1];
        float SegmentLength = EndDistance - StartDistance;

        if (SegmentLength <= KINDA_SMALL_NUMBER)
            continue;

        float Alpha = (TargetDistance - StartDistance) / SegmentLength;
        FVector Position = FMath::Lerp(CurveSamples[SegmentIndex], CurveSamples[SegmentIndex + 1], Alpha);

        DistanceSamples.Add(Position);
        DistanceSampleDistances.Add(TargetDistance);
    }

    DistanceSamples.Add(CurveSamples.Last());
    DistanceSampleDistances.Add(TotalSplineLength);
    */
}

#include "WaypointPathAsset.h"
void FWaypointEditorData::GetData(UMovementPathAsset* dataClass)
{
    if (dataClass == nullptr)
        return;

    dataClass->SavedPoints.Reset();
    dataClass->SavedWaypoints.Reset();

    const TArray<FWaypoint>& Waypoint = GetWaypointList();
    const TArray<FQuat>& SplineRotation = GetSplineRotationList();
    const TArray<FVector>& SplinePoint = GetSplinePointList();

    if (Waypoint.IsEmpty()) return;

    int length = FMath::Min(SplineRotation.Num(), SplinePoint.Num()); // 아마 왠만하면 같을텐데
    dataClass->SavedPoints.Reserve(length);
    dataClass->SavedWaypoints.Reserve(Waypoint.Num());

    for (const auto& item : Waypoint)
        dataClass->SavedWaypoints.Emplace(item.Position, item.Rotation.Quaternion(), 0.0);

    if (length == 0) return;

    dataClass->SavedPoints.Emplace(SplinePoint[0], SplineRotation[0], 0.0);
    for (int i = 1; i < length; i++)
    {
        double distance = (dataClass->SavedPoints.Last().Position - SplinePoint[i]).Length(); // DistanceSamples을 이용하면 이걸 안해도 될라나
        dataClass->SavedPoints.Emplace(SplinePoint[i], SplineRotation[i], distance);
    }
}

void FWaypointEditorData::SetData(UMovementPathAsset* dataClass)
{
    if (dataClass == nullptr || dataClass->SavedPoints.IsEmpty() || dataClass->SavedWaypoints.IsEmpty())
        return;

    Waypoints.Reset();
    DistanceSampleRotations.Reset();
    DistanceSamples.Reset();

    Waypoints.Reserve(dataClass->SavedWaypoints.Num());
    DistanceSampleRotations.Reserve(dataClass->SavedPoints.Num());
    DistanceSamples.Reserve(dataClass->SavedPoints.Num());

    for (const auto& item : dataClass->SavedWaypoints)
        Waypoints.Emplace(item.Position, FRotator(item.Rotation));

    for (const auto& item : dataClass->SavedPoints)
    {
        DistanceSamples.Add(item.Position);
        DistanceSampleRotations.Add(item.Rotation);
    }
}