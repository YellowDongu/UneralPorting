// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Defined.h"
#include "MainMenuUI.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UMainMenuUI : public UUserWidget
{
	GENERATED_BODY()
public:
	//DECLARE_MULTICAST_DELEGATE(FVoidDelegate);
	//DECLARE_DELEGATE(FVoidDelegate);conversion

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& InGeometry, float DeltaTime) override;
	////virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	void Select(void);
	void Deselect(void);
	void TransitLevel(void);

	void Active(void);
	void Deactive(void);

private:
	FStringListPanelController* CurrentListController{ nullptr };
	FStringListPanelController MainListController;
	FStringListPanelController MissionListController;
	
	FTimerHandle TimerHandle;

	bool bActive{ false };
	int Phase{ 0 };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* MainListCanvas{ nullptr };
	
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* MissionListCanvas{ nullptr };

	UPROPERTY() TObjectPtr<USoundBase> Selected{ nullptr };
	UPROPERTY() TObjectPtr<USoundBase> MenuChange{ nullptr };
	UPROPERTY() TObjectPtr<USoundBase> MenuCancel{ nullptr }; // 이렇게 쓰는 편이 좋나?
	UPROPERTY() TObjectPtr<USoundBase> MainBGM{ nullptr };
	

};
