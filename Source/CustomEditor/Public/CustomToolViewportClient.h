#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "WaypointEditorData.h"

class CUSTOMEDITOR_API FCustomToolViewportClient : public FEditorViewportClient
{
public:
    FCustomToolViewportClient(class FPreviewScene* InPreviewScene, const TSharedRef<SEditorViewport>& InViewport, TSharedPtr<class FWaypointEditorData> InWaypointData);

    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual void Draw(const class FSceneView* View, class FPrimitiveDrawInterface* PDI) override;
    virtual void MouseMove(FViewport* viewport, int32 X, int32 Y) override;
    bool GetMouseGroundPosition(FVector& OutPosition);
    int32 FindNearestWaypoint(const FVector& Position, const FVector& RayDirection);

    void AddWaypointAtMouse(void);

    bool HasSelectedWaypoint(void) const { return WaypointData->WaypointIndexValid(WaypointData->SelectedWaypointIndex); }
    FVector GetSelectedWaypointPosition(void) const { if (!HasSelectedWaypoint()) return FVector::ZeroVector; return WaypointData->GetWaypointList()[WaypointData->SelectedWaypointIndex].Position; }
    void SetSelectedWaypointPosition(const FVector& Position) { if (!HasSelectedWaypoint()) return; WaypointData->SetPointPosition(Position, WaypointData->SelectedWaypointIndex); Invalidate(); }
private:
    TSharedPtr<class FWaypointEditorData> WaypointData;

    //TArray<FVector> Waypoints;
    //int32 SelectedWaypointIndex = INDEX_NONE;

    bool bDraggingWaypoint = false;
    float SpawnDistance = 1000.0f;
    int Jump = 1;
    int DisplayDistance = 100.0f;
};