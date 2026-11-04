// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleUI.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UTitleUI : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& myGeometry, float deltaTime) override;

private:
	float Timer{ 0.0f };
	int Flag{-1};
	bool bPressed{ false };

	FTimerHandle TimerHandle;

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCanvasPanel* TitleCanvas{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCanvasPanel* PreTitleCanvas{ nullptr };
	
	
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UImage* Title{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UImage* PreTitle1{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* NextText{ nullptr };

	UPROPERTY() TObjectPtr<USoundBase> TurnToMainMenu{ nullptr };
	TWeakObjectPtr<UStageChangeUIUnit> FrontUI{ nullptr };
};
