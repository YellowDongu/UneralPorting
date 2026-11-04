// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RadarTargetUI.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API URadarTargetUI : public UUserWidget
{
	GENERATED_BODY()
public:
	void Update(const FVector2D& position, float angle, class AVehicleBase* target);

	void Attach(class UCanvasPanel* Canvas);
	void ChangeImage(class AVehicleBase* target);


	static float GetSize() { return Size; }
	inline void SetColor(const FLinearColor& NewColor);
private:
	int Mask{ -1 }, TeamMask{ -1 };
	static float Size;


	TWeakObjectPtr<class UCanvasPanelSlot> CanvasSlot{ nullptr };
	class UImage* Current{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UImage* AirTGT{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UImage* Air{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UImage* GroundTGT{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UImage* Ground{ nullptr };
	
	
};
