#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "WaypointEditorData.h"

template<typename ItemType>
class SListView;

class CUSTOMEDITOR_API SWaypointList : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SWaypointList) {}
        SLATE_ARGUMENT(TSharedPtr<FWaypointEditorData>, WaypointData)
    SLATE_END_ARGS()
    virtual ~SWaypointList();

    void Construct(const FArguments& InArgs);
    void MoveSelectedWaypoint(int32 Direction);
    void DeleteSelectedWaypoint(void);
    void OnDataChanged(void);

private:
    void RebuildItems(void);
    void OnSelectionChanged(TSharedPtr<int32> Item, ESelectInfo::Type SelectInfo);
    TSharedRef<ITableRow> GenerateRow(TSharedPtr<int32> Item, const TSharedRef<STableViewBase>& OwnerTable);

    TSharedPtr<FWaypointEditorData> WaypointData;
    TArray<TSharedPtr<int32>> Items;
    TSharedPtr<SListView<TSharedPtr<int32>>> ListView;
    FDelegateHandle DataChangedHandle;

    bool bUpdatingSelection = false;
};