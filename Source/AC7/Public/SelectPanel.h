// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Defined.h"
#include "SelectPanel.generated.h"

/**
 * 
 */

UCLASS()
class AC7_API USelectPanel : public UUserWidget
{
	GENERATED_BODY()
public:
	//DECLARE_MULTICAST_DELEGATE(FVoidDelegate);
	DECLARE_DELEGATE(FVoidDelegate);
	DECLARE_DELEGATE_OneParam(FChangeDisplay, EVehicleID);

	virtual void NativeTick(const FGeometry& InGeometry, float DeltaTime);
	//virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeConstruct(void) override;
	UFUNCTION(BlueprintCallable) void LinkDisplayCase(class ADisplayCase* target);
	
	void Select(void);
	void Deselect(void);
	void ReturnMenu(void);
	void EnterStage(void);
	void UpdateWeaponSelection(void);

	void Active(void) { bActive = true; }
	void Deactive(void) { bActive = false; }

	//void LinkFunction(const FVoidDelegate::FDelegate& Delegate) { EnterMission.Add(Delegate); }
	void LinkFunction(FVoidDelegate Delegate) { EnterMission = MoveTemp(Delegate); }

private:
	FStringListPanelController VehicleListController;
	FStringListPanelController WeaponListController;
	FStringListPanelController* CurrentListController{ nullptr };

	FVoidDelegate EnterMission;
	FChangeDisplay ChangeSelectDelegate;

	bool bActive{ false };
	int Phase{ 0 };

	const TArray<TWeakObjectPtr<AVehicleBase>>* SelectableVehicleList{nullptr};

	class UCanvasPanel* SelectedBar{ nullptr };
	class UCanvasPanel* ListCanvas{ nullptr };
	class UVerticalBox* ListPanel{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* VehicleListSelectedBar{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* WeaponListSelectedBar{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* VehicleListCanvas{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UVerticalBox* VehicleListPanel{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* WeaponListCanvas{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UVerticalBox* WeaponListPanel{ nullptr };
	
	UPROPERTY() TObjectPtr<USoundBase> Selected{ nullptr };
	UPROPERTY() TObjectPtr<USoundBase> MenuChange{ nullptr };
	UPROPERTY() TObjectPtr<USoundBase> MenuCancel{ nullptr };
	UPROPERTY() TObjectPtr<USoundBase> AircraftSelected{ nullptr };
	UPROPERTY() TObjectPtr<USoundBase> MainBGM{ nullptr };
};
