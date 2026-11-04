#include "CustomToolViewportClient.h"

#include "WaypointEditorData.h"
#include "PreviewScene.h"
#include "SEditorViewport.h"
//#include "PrimitiveDrawingUtils.h"
//#include "PrimitiveDrawInterface.h"
#include "SceneManagement.h"
#include "InputCoreTypes.h"
#include "SceneView.h"

FCustomToolViewportClient::FCustomToolViewportClient(FPreviewScene* InPreviewScene, const TSharedRef<SEditorViewport>& InViewport, TSharedPtr<FWaypointEditorData> InWaypointData) : FEditorViewportClient(nullptr, InPreviewScene, InViewport), WaypointData(InWaypointData)
{
    SetViewportType(LVT_Perspective); // 원근 투영 뷰

    SetViewLocation(FVector(500.0f, 500.0f, 300.0f));
    SetViewRotation(FRotator(-20.0f, -135.0f, 0.0f));

    SetRealtime(true); // 뷰포트 실시간 갱신

    WaypointData->OnChanged.AddLambda([this]() -> void
                                      {
                                          const TArray<FVector>& PointList = WaypointData->GetSplinePointList();
                                          Jump = 1;
                                          if (PointList.Num() > 1)
                                              Jump = FMath::Max(1, FMath::FloorToInt(DisplayDistance / (PointList[0] - PointList[1]).Length()));
                                      });
}

bool FCustomToolViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
    constexpr float MinSpawnDistance = 100.0f;
    constexpr float MaxSpawnDistance = 100000.0f;

    if (EventArgs.Key == EKeys::MouseScrollUp && EventArgs.Event == IE_Pressed)
    {
        SpawnDistance = FMath::Clamp(SpawnDistance / 1.2f, MinSpawnDistance, MaxSpawnDistance);
        Invalidate();
    }
    else if (EventArgs.Key == EKeys::MouseScrollDown && EventArgs.Event == IE_Pressed)
    {
        SpawnDistance = FMath::Clamp(SpawnDistance * 1.2f, MinSpawnDistance, MaxSpawnDistance);
        Invalidate();
    }

    // 왼쪽 클릭: 점 선택 또는 생성
    if (EventArgs.Key == EKeys::LeftMouseButton)
    {
        if (EventArgs.Event == IE_Released)
        {
            bDraggingWaypoint = false;
            return true;
        }
        else if (EventArgs.Event == IE_Pressed)
        {

        FViewportCursorLocation Cursor = GetCursorWorldLocationFromMousePos();

        FVector RayOrigin = Cursor.GetOrigin();
        FVector RayDirection = Cursor.GetDirection().GetSafeNormal();

        // 기존 웨이포인트 선택 시도
        int32 NearestIndex = FindNearestWaypoint(RayOrigin, RayDirection);

        if (NearestIndex != INDEX_NONE)
            WaypointData->SelectWaypoint(NearestIndex);
        else
        {
            // 레이 방향으로 지정 거리만큼 떨어진 위치에 생성
            FVector NewPosition = RayOrigin + RayDirection * SpawnDistance;

            WaypointData->AddWaypoint(NewPosition);
            //WaypointData->Waypoints.Add(NewPosition);
            WaypointData->SelectedWaypointIndex = WaypointData->GetWaypointList().Num() - 1;
        }
        bDraggingWaypoint = true;
        }
        //else if (EventArgs.Event == IE_DoubleClick)
        //{
        //
        //    bDraggingWaypoint = true;
        //}
        Invalidate();
        return true;
    }

    // 선택한 웨이포인트 삭제
    if (EventArgs.Key == EKeys::Delete && EventArgs.Event == IE_Pressed)
    {
        //if (WaypointData->Waypoints.IsValidIndex(WaypointData->SelectedWaypointIndex))
        if (WaypointData->HasSelection())
        {
            //WaypointData->Waypoints.RemoveAt(WaypointData->SelectedWaypointIndex);
            WaypointData->RemoveSelectedWaypoint();
            WaypointData->SelectedWaypointIndex = INDEX_NONE;

            Invalidate();
        }
    }

    return FEditorViewportClient::InputKey(EventArgs);
}

void FCustomToolViewportClient::MouseMove(FViewport* viewport, int32 X, int32 Y)
{
    FEditorViewportClient::MouseMove(viewport, X, Y);

    if (!bDraggingWaypoint)
        return;

    //if (!WaypointData->Waypoints.IsValidIndex(WaypointData->SelectedWaypointIndex))
    if (!WaypointData->HasSelection())
    {
        bDraggingWaypoint = false;
        return;
    }

    FVector GroundPosition;
    if (GetMouseGroundPosition(GroundPosition))
    {
        WaypointData->SetSelectedPosition(GroundPosition);
        //WaypointData->Waypoints[WaypointData->SelectedWaypointIndex] = GroundPosition;
        Invalidate();
    }
}

bool FCustomToolViewportClient::GetMouseGroundPosition(FVector& OutPosition)
{
    FViewportCursorLocation Cursor = GetCursorWorldLocationFromMousePos();
    FVector RayOrigin = Cursor.GetOrigin();
    FVector RayDirection = Cursor.GetDirection();

    if (FMath::IsNearlyZero(RayDirection.Z))
        return false;

    const float Distance = -RayOrigin.Z / RayDirection.Z;

    if (Distance < 0.0f)
        return false;

    const FVector HitPosition = RayOrigin + RayDirection * Distance;
    OutPosition = FVector(HitPosition.X, HitPosition.Y, 0.0f);
    return true;
}

int32 FCustomToolViewportClient::FindNearestWaypoint(const FVector& RayOrigin, const FVector& RayDirection)
{
    constexpr float SelectRadius = 50.0f;
    constexpr float SelectRadiusSq = FMath::Square(SelectRadius);

    int32 NearestIndex = INDEX_NONE;
    float NearestDistanceSq = SelectRadiusSq;

    const auto& Waypoints = WaypointData->GetWaypointList();
    for (int32 i = 0; i < Waypoints.Num(); ++i)
    {
        const FVector ToWaypoint = Waypoints[i].Position - RayOrigin;
        // 레이 방향으로 투영한 거리
        const float AlongRay = FVector::DotProduct(ToWaypoint, RayDirection);

        // 카메라 뒤쪽의 점은 선택하지 않음
        if (AlongRay < 0.0f)
            continue;

        // 레이에서 해당 점까지 가장 가까운 위치
        const FVector ClosestPoint = RayOrigin + RayDirection * AlongRay;
        const float DistanceSq = FVector::DistSquared(Waypoints[i].Position, ClosestPoint);

        if (DistanceSq <= NearestDistanceSq)
        {
            NearestDistanceSq = DistanceSq;
            NearestIndex = i;
        }
    }

    return NearestIndex;
    /*
    constexpr float SelectRadius = 50.0f;
    constexpr float SelectRadiusSq = FMath::Square(SelectRadius);

    int32 NearestIndex = INDEX_NONE;
    float NearestDistanceSq = SelectRadiusSq;

    for (int32 i = 0; i < Waypoints.Num(); ++i)
    {
        const float DistanceSq = FVector::DistSquared2D(Waypoints[i], Position);

        if (DistanceSq <= NearestDistanceSq)
        {
            NearestDistanceSq = DistanceSq;
            NearestIndex = i;
        }
    }

    return NearestIndex;
    */
}

void FCustomToolViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    FEditorViewportClient::Draw(View, PDI);

    constexpr static float GridSpacing = 100.0f;
    constexpr static int32 GridCount = 20;
    constexpr static float GridZ = -1.0f;
    constexpr static float GridExtent = GridSpacing * GridCount;
    
    constexpr static FLinearColor GridColor(0.25f, 0.28f, 0.32f); // 일반 그리드
    constexpr static FLinearColor XAxisColor(0.9f, 0.2f, 0.2f); // Y축 색상
    constexpr static FLinearColor YAxisColor(0.2f, 0.8f, 0.3f); // X축 


    for (int32 i = -GridCount; i <= GridCount; ++i)
    {
        float Offset = i * GridSpacing;
        float Thickness = (i == 0) ? 2.0f : 0.5f;
        FLinearColor XColor = (i == 0) ? XAxisColor : GridColor;
        FLinearColor YColor = (i == 0) ? YAxisColor : GridColor;

        PDI->DrawLine(FVector(-GridExtent, Offset, GridZ), FVector(GridExtent, Offset, GridZ), YColor, SDPG_World, Thickness); // X 방향으로 뻗는 선
        PDI->DrawLine(FVector(Offset, -GridExtent, GridZ), FVector(Offset, GridExtent, GridZ), XColor, SDPG_World, Thickness); // Y 방향으로 뻗는 선
    }

    if (!WaypointData.IsValid())
        return;

    const TArray<FVector>& PointList = WaypointData->GetSplinePointList();
    for (int32 i = 1; i < PointList.Num(); ++i)
        PDI->DrawLine(PointList[i - 1], PointList[i], FLinearColor::Green, SDPG_Foreground, 2.0f);

    const TArray<FQuat>& Rots = WaypointData->GetSplineRotationList();
    for (int32 i = 0; i < PointList.Num(); i += Jump)   // spacing이 1이면 샘플이 많으니 간격을 띄워서
        DrawCoordinateSystem(PDI, PointList[i], FRotator(Rots[i]), 30.f, SDPG_Foreground);

    const auto& Waypoints = WaypointData->GetWaypointList();
    for (int32 i = 0; i < Waypoints.Num(); ++i)
    {
        const FVector& Position = Waypoints[i].Position;
        bool bSelected = (i == WaypointData->SelectedWaypointIndex);
        FLinearColor PointColor = bSelected ? FLinearColor(0.1f, 0.8f, 1.0f) : FLinearColor(1.0f, 0.75f, 0.1f); // 선택: 하늘색 / 일반: 노란색
        float PointSize = bSelected ? 18.0f : 12.0f;

        PDI->DrawPoint(Position, PointColor, PointSize, SDPG_World);
        if(bSelected)
            DrawCoordinateSystem(PDI, Position, FRotator(Waypoints[i].Rotation), 100.f, SDPG_Foreground, 5.0f);

        if (i > 0)
            PDI->DrawLine(Waypoints[i - 1].Position, Position, FLinearColor(1.0f, 0.75f, 0.1f), SDPG_World, 2.0f);
    }
}

void FCustomToolViewportClient::AddWaypointAtMouse(void)
{
    FViewportCursorLocation Cursor = GetCursorWorldLocationFromMousePos();
    FVector RayOrigin = Cursor.GetOrigin();
    FVector RayDirection = Cursor.GetDirection();
    
    if (FMath::IsNearlyZero(RayDirection.Z)) // XY 평면(Z=0)과 레이가 평행한 경우
        return;

    const float Distance = -RayOrigin.Z / RayDirection.Z;
    if (Distance < 0.0f)
        return;

    const FVector HitPosition = RayOrigin + RayDirection * Distance;
    WaypointData->AddWaypoint(FVector(HitPosition.X, HitPosition.Y, 0.0f));

    Invalidate(); // 뷰포트 다시 그리기 요청
}