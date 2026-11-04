// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IFFHUD.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UIFFHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FRetriveUI, UIFFHUD*);
	FRetriveUI RetriveUI;

	UIFFHUD(const FObjectInitializer& ObjectInitializer);
	void StartTrack(class AVehicleBase* vehicle, class AAircraft* player, class UCanvasPanelSlot* AttachedCanvasSlot);
	void Update(float DeltaTime);

	void SelectTarget();
	void DeselectTarget();

	void Active(bool active);
	void SetColor(const FLinearColor& color) { Color = color; SetColor(); }

	bool IsSelected() const { return bIsSelected; }
	bool IsActive() const { return bIsActive; }
	const TWeakObjectPtr<class AVehicleBase>& GetTargetVehicle() { return TargetVehicle; }
private:
	void SetColor();
	void SetNegative(const FWeaponMask& mask);

	bool bIsActive{ false };
	bool bIsSelected{ false };
	bool bIsVisible{ false };
	bool bIsBoxVisible{ false };
	int Layer{ -1 };

	FLinearColor Color;

	TWeakObjectPtr<class AVehicleBase> TargetVehicle{ nullptr };
	TWeakObjectPtr<class AAircraft> Player{ nullptr };
	TWeakObjectPtr<class UCanvasPanelSlot> CanvasSlot{ nullptr };
	class UImage* Box{ nullptr };
	class UImage* Box_NG{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCanvasPanel* BaseCanvas{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UImage* Air{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UImage* Air_NG{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UImage* Ground{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UImage* Ground_NG{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* TGTText{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* DistanceText{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* TypeNameText{ nullptr };
};
