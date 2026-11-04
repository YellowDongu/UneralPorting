#include "WaypointListWidget.h"

#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Text/STextBlock.h"
/*
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
*/

void SWaypointList::Construct(const FArguments& InArgs)
{
    WaypointData = InArgs._WaypointData;

    RebuildItems();
    
    ChildSlot
        [
            SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight().Padding(4.0f)
                [
                    SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1.0f)
                        [
                            SNew(SButton).Text(FText::FromString(TEXT("Move Up")))
                                .OnClicked_Lambda([this]()
                                                  {
                                                      MoveSelectedWaypoint(-1);
                                                      return FReply::Handled();
                                                  })
                        ]

                    + SHorizontalBox::Slot().FillWidth(1.0f)
                        [
                            SNew(SButton).Text(FText::FromString(TEXT("Move Down")))
                                .OnClicked_Lambda([this]()
                                                  {
                                                      MoveSelectedWaypoint(1);
                                                      return FReply::Handled();
                                                  })
                        ]

                    + SHorizontalBox::Slot().FillWidth(1.0f)
                        [
                            SNew(SButton).Text(FText::FromString(TEXT("Delete")))
                                .OnClicked_Lambda([this]()
                                                  {
                                                      DeleteSelectedWaypoint();
                                                      return FReply::Handled();
                                                  })
                        ]
                ]

            + SVerticalBox::Slot().FillHeight(1.0f)
                [
                    SAssignNew(ListView, SListView<TSharedPtr<int32>>).ListItemsSource(&Items)
                        .OnGenerateRow(this, &SWaypointList::GenerateRow)
                        .OnSelectionChanged(this, &SWaypointList::OnSelectionChanged)
                ]
        ];
        

    /*
    ChildSlot
        [
            SAssignNew(ListView, SListView<TSharedPtr<int32>>).ListItemsSource(&Items)
            //SNew(SListView<TSharedPtr<int32>>).ListItemsSource(&Items)
                .OnGenerateRow(this, &SWaypointList::GenerateRow)
                .OnSelectionChanged(this, &SWaypointList::OnSelectionChanged)
        ];

    //ListView = StaticCastSharedPtr<SListView<TSharedPtr<int32>>>(ChildSlot.GetWidget());
    */

    if (WaypointData.IsValid())
        DataChangedHandle = WaypointData->OnChanged.AddSP(this, &SWaypointList::OnDataChanged);

    OnDataChanged();
}

SWaypointList::~SWaypointList()
{
    if (WaypointData.IsValid() && DataChangedHandle.IsValid())
        WaypointData->OnChanged.Remove(DataChangedHandle);
}

void SWaypointList::RebuildItems()
{
    Items.Reset();

    if (!WaypointData.IsValid())
        return;

    for (int32 Index = 0; Index < WaypointData->GetWaypointList().Num(); ++Index)
        Items.Add(MakeShared<int32>(Index));
}

void SWaypointList::OnDataChanged()
{
    if (!WaypointData.IsValid() || !ListView.IsValid())
        return;

    TSharedPtr<int32> SelectedItem;
    bUpdatingSelection = true;

    RebuildItems();
    ListView->RequestListRefresh();
    
    if (WaypointData->HasSelection())
        SelectedItem = Items[WaypointData->SelectedWaypointIndex];

    ListView->SetSelection(SelectedItem);
    bUpdatingSelection = false;
}

void SWaypointList::OnSelectionChanged(TSharedPtr<int32> Item, ESelectInfo::Type SelectInfo)
{
    if (bUpdatingSelection || !WaypointData.IsValid())
        return;

    if (Item.IsValid())
        WaypointData->SelectWaypoint(*Item);
}

TSharedRef<ITableRow> SWaypointList::GenerateRow(TSharedPtr<int32> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    FString Label = Item.IsValid() ? FString::Printf(TEXT("Waypoint %d"), *Item + 1) : TEXT("Invalid Waypoint");

    return SNew(STableRow<TSharedPtr<int32>>, OwnerTable)[SNew(STextBlock).Text(FText::FromString(Label))];
}

void SWaypointList::MoveSelectedWaypoint(int32 Direction)
{
    if (!WaypointData.IsValid() || !WaypointData->HasSelection())
        return;

    int32 FromIndex = WaypointData->SelectedWaypointIndex;

    int32 ToIndex = FromIndex + Direction;

    if (!WaypointData->GetWaypointList().IsValidIndex(ToIndex))
        return;

    WaypointData->MoveWaypoint(FromIndex, ToIndex);
}

void SWaypointList::DeleteSelectedWaypoint()
{
    if (!WaypointData.IsValid() || !WaypointData->HasSelection())
        return;

    WaypointData->RemoveWaypoint(WaypointData->SelectedWaypointIndex);
}