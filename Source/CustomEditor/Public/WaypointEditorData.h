#pragma once

#include "CoreMinimal.h"

DECLARE_MULTICAST_DELEGATE(FOnWaypointDataChanged);

struct FWaypoint
{
    FWaypoint(const FVector& position = FVector::ZeroVector, const FRotator& rotation = FRotator::ZeroRotator) : Position(position), Rotation(rotation) {}

    FVector Position = FVector::ZeroVector;
    FRotator Rotation = FRotator::ZeroRotator;
};

class CUSTOMEDITOR_API FWaypointEditorData
{
public:
    friend class UWaypointPathAsset;
    FWaypointEditorData();
    ~FWaypointEditorData() = default;

    inline bool HasSelection(void) const { return Waypoints.IsValidIndex(SelectedWaypointIndex); }
    inline const FVector& GetSelectedPosition(void) const { return HasSelection() ? Waypoints[SelectedWaypointIndex].Position : FVector::ZeroVector; }
    inline const FRotator& GetSelectedRotation(void) const { return HasSelection() ? Waypoints[SelectedWaypointIndex].Rotation : FRotator::ZeroRotator; }

    void SelectWaypoint(int32 Index);
    void RemoveSelectedWaypoint(void);
    void AddWaypoint(const FVector& Position);
    void RemoveWaypoint(int32 Index);
    void MoveWaypoint(int32 FromIndex, int32 ToIndex);
    void RebuildSpline(void);
    void BuildSpline(void);

    void GetData(class UMovementPathAsset* dataClass);
    void SetData(class UMovementPathAsset* dataClass);

    FOnWaypointDataChanged OnChanged;

    inline FVector EvaluateCatmullRom(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T);
    void AppendAdaptiveSamples(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T0, float T1, const FVector& Start, const FVector& End, float MaxChordLength, float FlatnessTolerance, int32 Depth, TArray<FVector>& OutSamples);


    void SetSelectedPosition(const FVector& Position);
    void SetSelectedRotation(const FRotator& Rotation);
    void OffsetAllPositions(const FVector& Delta);
    void ScaleAllPositions(const FVector& Scale);
    void SetPointPosition(const FVector& Position, int32 index) { if (Waypoints.IsValidIndex(index)) Waypoints[index].Position = Position; }

    inline bool WaypointIndexValid(int32 index) const { return Waypoints.IsValidIndex(index); }
    inline bool SplineIndexValid(int32 index) const { return SplineSamples.IsValidIndex(index); }
    //const TArray<FVector>& GetWaypointList(void) { return Waypoints; }
    const TArray<FWaypoint>& GetWaypointList(void) { return Waypoints; }
    const TArray<FVector>& GetSplinePointList(void) { return DistanceSamples; }
    const TArray<FQuat>& GetSplineRotationList(void) { return DistanceSampleRotations; }
    int32 GetSelectedIndex(void) { return SelectedWaypointIndex; }
    void SetSelectedIndex(int32 index) { if (Waypoints.IsValidIndex(index)) SelectedWaypointIndex = index; }

    int32 SubdivisionsPerSegment = 10;
    int32 SelectedWaypointIndex = INDEX_NONE;

private:

    //TArray<FVector> Waypoints;
    TArray<FWaypoint> Waypoints;
    TArray<FQuat> DistanceSampleRotations;

    TArray<FWaypoint> LerpedWaypoints;


    TArray<FVector> SplineSamples;

    float DistanceSampleSpacing = 1.0f;
    TArray<FVector> DistanceSamples;
    TArray<float> DistanceSampleDistances;
    float TotalSplineLength = 0.0f;
};