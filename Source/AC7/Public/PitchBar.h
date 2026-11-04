// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PitchBar.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UPitchBar : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetNumber(int number);
	UPitchBar* SetUp(class UCanvasPanel* AttachedCanvasSlot);
	UPitchBar* SetUp();

	void SetPosition(const FVector2D& value);
	void SetUIColor(FLinearColor value);
	void SetSideTextColor(FSlateColor value);

private:
	UPROPERTY(EditAnywhere, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	class UTextBlock* Number{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	class UImage* Image{ nullptr };

	TWeakObjectPtr<class UCanvasPanelSlot> CanvasSlot{ nullptr };
};
