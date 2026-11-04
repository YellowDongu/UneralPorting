// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatusUI.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UStatusUI : public UUserWidget
{
	GENERATED_BODY()
public:
	void BindPlayer(class AAircraft* vehicle, class APlayerPilot* player);
	//virtual void NativePreConstruct() override;

private:
	TWeakObjectPtr<class AAircraft> Player{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* BaseCanvas{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* TextPanel{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* GaugePanel{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UGaugeUI* Gauge{ nullptr };

};
