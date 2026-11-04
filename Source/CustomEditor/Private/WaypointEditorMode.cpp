#include "WaypointEditorMode.h"

const FEditorModeID FWaypointEditorMode::EM_WaypointEditorModeId =TEXT("EM_WaypointEditorMode");

FWaypointEditorMode::FWaypointEditorMode()
{
}

FWaypointEditorMode::~FWaypointEditorMode()
{
}

void FWaypointEditorMode::Enter()
{
    FEdMode::Enter();

    // 에디터 모드 진입 시 초기화
}

void FWaypointEditorMode::Exit()
{
    // 에디터 모드 종료 시 정리

    FEdMode::Exit();
}