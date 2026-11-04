#pragma once

#include "CoreMinimal.h"
#include "WaypointEditorData.h"
#include "MovementPathAsset.h"
#include "WaypointPathAsset.generated.h"

UCLASS(BlueprintType)
class CUSTOMEDITOR_API UWaypointPathAsset : public UMovementPathAsset
{
    GENERATED_BODY()
public:
    //virtual void PostLoad() override { Super::PostLoad(); }

    void GetData(class FWaypointEditorData* dataClass);
    void SetData(class FWaypointEditorData* dataClass);

    static void Save(const FString& PackageName, class FWaypointEditorData* dataClass);
    static void Load(const FString& ObjectPath, class FWaypointEditorData* dataClass);

};