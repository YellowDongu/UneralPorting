#include "WaypointPathAsset.h"

#include "Kismet/GameplayStatics.h"
//#include "Engine/LevelStreamingDynamic.h"
//#include "Tasks/Task.h"
//#include "Async/TaskGraphInterfaces.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "FileHelpers.h"

void UWaypointPathAsset::GetData(FWaypointEditorData* dataClass)
{
    if (dataClass == nullptr)
        return;

    SavedPoints.Reset();
    SavedWaypoints.Reset();

    const TArray<FWaypoint>& Waypoint = dataClass->GetWaypointList();
    const TArray<FQuat>& SplineRotation = dataClass->GetSplineRotationList();
    const TArray<FVector>& SplinePoint = dataClass->GetSplinePointList();

    if (Waypoint.IsEmpty()) return;

    int length = FMath::Min(SplineRotation.Num(), SplinePoint.Num()); // 아마 왠만하면 같을텐데
    SavedPoints.Reserve(length);
    SavedWaypoints.Reserve(Waypoint.Num());

    for (const auto& item : Waypoint)
        SavedWaypoints.Emplace(item.Position, item.Rotation.Quaternion(), 0.0);

    if (length == 0) return;

    SavedPoints.Emplace(SplinePoint[0], SplineRotation[0], 0.0);
    for (int i = 1; i < length; i++)
    {
        double distance = (SavedPoints.Last().Position - SplinePoint[i]).Length(); // DistanceSamples을 이용하면 이걸 안해도 될라나
        SavedPoints.Emplace(SplinePoint[i], SplineRotation[i], distance);
    }
}

void UWaypointPathAsset::SetData(FWaypointEditorData* dataClass)
{
    if (dataClass == nullptr || SavedWaypoints.IsEmpty())
        return;

    dataClass->Waypoints.Reset();
    dataClass->DistanceSampleRotations.Reset();
    dataClass->DistanceSamples.Reset();

    dataClass->Waypoints.Reserve(SavedWaypoints.Num());
    dataClass->DistanceSampleRotations.Reserve(SavedPoints.Num());
    dataClass->DistanceSamples.Reserve(SavedPoints.Num());

    for (const auto& item : SavedWaypoints)
        dataClass->Waypoints.Emplace(item.Position, FRotator(item.Rotation));

    for (const auto& item : SavedPoints)
    {
        dataClass->DistanceSamples.Add(item.Position);
        dataClass->DistanceSampleRotations.Add(item.Rotation);
    }
}


void UWaypointPathAsset::Save(const FString& PackageName, class FWaypointEditorData* dataClass)
{
    UMovementPathAsset* Asset = GetOrCreateAsset(PackageName);
    Asset->Modify();
    dataClass->GetData(Asset);
    Asset->MarkPackageDirty();

    TArray<UPackage*> Packages{ Asset->GetOutermost() };
    FEditorFileUtils::PromptForCheckoutAndSave(Packages, false, false);
}

void UWaypointPathAsset::Load(const FString& ObjectPath, class FWaypointEditorData* dataClass)   // 예: "/Game/EditorOnly/Waypoints/Path01.Path01"
{
    UWaypointPathAsset* Asset = StaticCast<UWaypointPathAsset*>(LoadObject<UMovementPathAsset>(nullptr, *ObjectPath));
    if (Asset == nullptr)
        return;

    //dataClass->SetData(Asset);
    Asset->SetData(dataClass);
}