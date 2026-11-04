#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
//CUSTOMEDITOR_API
class CUSTOMEDITOR_API FWaypointEditorMode : public FEdMode
{
public:
    static const FEditorModeID EM_WaypointEditorModeId;

    FWaypointEditorMode();
    virtual ~FWaypointEditorMode() override;

    virtual void Enter() override;
    virtual void Exit() override;
};