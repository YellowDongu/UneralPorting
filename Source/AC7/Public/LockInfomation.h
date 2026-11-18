// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Defined.h"
/**
 * 
 */
class AC7_API LockSet
{
public:
	LockSet() = default;
	~LockSet() = default;

	struct IndicatorCreationData
	{
		TWeakObjectPtr<class AMissile> Weapon{ nullptr };
		class UCanvasPanel* CanvasPanel{ nullptr };
		TWeakObjectPtr<class UFireControlSystem> FCS{ nullptr };
		TSubclassOf<class UUserWidget> LockBluePrint;
		TSubclassOf<class UUserWidget> SpecialLockBluePrint;
		TSubclassOf<AActor> RangeIndicatorBluePrint;
	};

	virtual void Initialize(IndicatorCreationData& data) {}
	virtual void Update(float deltaTime, FTargetInfo* target) {}
	virtual void SetActive(bool value) {}
	virtual void SetColor(const FLinearColor& color) {}

	static TSharedPtr<LockSet> Create(IndicatorCreationData& data);

protected:
	FLinearColor CurrentColor;
	bool bIsActive{ true }, bIsLock{ false };
};

class AC7_API LockIndicator : public LockSet
{
public:
	LockIndicator() = default;
	virtual ~LockIndicator() = default;

	virtual void Initialize(IndicatorCreationData& data) override;
	void Initialize(class UCanvasPanel* canvasPanel, TSubclassOf<class UUserWidget>& LockBluePrint);
	virtual void Update(float deltaTime, FTargetInfo* target) override;
	virtual void SetActive(bool value) override;
	virtual void SetColor(const FLinearColor& color) override;

	void TraceTarget(FTargetInfo& target);
	void StartTraceTarget(FTargetInfo& target);

protected:
	FVector2D InitialPosition{ FVector2D::ZeroVector };

	TWeakObjectPtr<class AVehicleBase> Target{ nullptr };
	TObjectPtr<class UUserWidget> LockRectangle{ nullptr };
	TWeakObjectPtr<class UCanvasPanelSlot> CanvasSlot{ nullptr };
	TWeakObjectPtr<class APlayerController> PlayerController{ nullptr };
};

class AC7_API SAAMLockIndicator : public LockIndicator
{
public:
	SAAMLockIndicator() = default;
	virtual ~SAAMLockIndicator() = default;

	virtual void Initialize(IndicatorCreationData& data) override;
	virtual void Update(float deltaTime, FTargetInfo* target) override;
	virtual void SetActive(bool value) override;
	virtual void SetColor(const FLinearColor& color) override;

private:
	TWeakObjectPtr<class UFireControlSystem> FCS{ nullptr };
	TObjectPtr<AActor> RangeIndicator{ nullptr };
	TObjectPtr<AActor> Owner{ nullptr };
	TObjectPtr<class UWidgetComponent> RangeIndicatorUI{ nullptr };
};

class AC7_API BombIndicator : public LockSet
{
public:
	BombIndicator() = default;
	virtual ~BombIndicator() = default;

	virtual void Initialize(IndicatorCreationData& data) override;
	void Initialize(TWeakObjectPtr<class UFireControlSystem> fcs, TSubclassOf<AActor>& RangeIndicatorBluePrint, double area);

	virtual void Update(float deltaTime, FTargetInfo* target) override;
	virtual void SetActive(bool value) override;
	virtual void SetColor(const FLinearColor& color) override;

	void DisplayExpactPosition();

	DECLARE_DELEGATE_RetVal_OneParam(bool, FCalculateBombLandingPosition, FVector&);
	FCalculateBombLandingPosition CalculateBombLandingPosition;

private:
	//FVector2D InitialPosition{ FVector2D::ZeroVector };
	TWeakObjectPtr<class UFireControlSystem> FCS{ nullptr };
	TObjectPtr<AActor> RangeIndicator{ nullptr };
	TObjectPtr<class UWidgetComponent> RangeIndicatorUI{ nullptr };
	//TWeakObjectPtr<class UCanvasPanelSlot> CanvasSlot{ nullptr };
};

class AC7_API LockableBombIndicator : public LockSet
{
public:
	LockableBombIndicator() = default;
	virtual ~LockableBombIndicator() = default;

	virtual void Initialize(IndicatorCreationData& data) override;
	void Initialize(TWeakObjectPtr<class UFireControlSystem> fcs, class UCanvasPanel* canvasPanel, TSubclassOf<class UUserWidget>& LockBluePrint, TSubclassOf<AActor>& RangeIndicatorBluePrint, double area);
	virtual void Update(float deltaTime, FTargetInfo* target) override;
	virtual void SetActive(bool value) override;
	virtual void SetColor(const FLinearColor& color) override;

	void TraceTarget(FTargetInfo& target);
	void StartTraceTarget(FTargetInfo& target);
private:
	FVector2D InitialPosition{ FVector2D::ZeroVector };
	TWeakObjectPtr<class AVehicleBase> Target{ nullptr };
	TObjectPtr<UUserWidget> LockRectangle{ nullptr };
	TWeakObjectPtr<class UCanvasPanelSlot> CanvasSlot{ nullptr };

	TWeakObjectPtr<class APlayerController> PlayerController;


	//FVector2D InitialPosition{ FVector2D::ZeroVector };
	TWeakObjectPtr<class UFireControlSystem> FCS{ nullptr };
	TObjectPtr<AActor> RangeIndicator{ nullptr };
	TObjectPtr<class UWidgetComponent> RangeIndicatorUI{ nullptr };
	//TWeakObjectPtr<class UCanvasPanelSlot> CanvasSlot{ nullptr };

};

/*
class AC7_API LockableBombIndicator : public LockSet
{
public:
	LockableBombIndicator() = default;
	virtual ~LockableBombIndicator() = default;

	void Initialize(class UCanvasPanel* canvasPanel, TSubclassOf<class UUserWidget>& LockBluePrint, TSubclassOf<AActor>& RangeIndicatorBluePrint, double area);
	virtual void Update(float deltaTime, FTargetInfo* target) override;
	virtual void SetActive(bool value) override;
	virtual void SetColor(const FLinearColor& color) override;

	void TraceTarget(FTargetInfo& target);
	void StartTraceTarget(FTargetInfo& target);
private:
	FVector2D InitialPosition{ FVector2D::ZeroVector };
	TWeakObjectPtr<class AVehicleBase> Target{ nullptr };
	TObjectPtr<UUserWidget> LockRectangle{ nullptr };
	TWeakObjectPtr<class UCanvasPanelSlot> CanvasSlot{ nullptr };

	TWeakObjectPtr<class APlayerController> PlayerController;


	//FVector2D InitialPosition{ FVector2D::ZeroVector };
	TWeakObjectPtr<class UFireControlSystem> FCS{ nullptr };
	TObjectPtr<AActor> RangeIndicator{ nullptr };
	TObjectPtr<class UWidgetComponent> RangeIndicatorUI{ nullptr };
	//TWeakObjectPtr<class UCanvasPanelSlot> CanvasSlot{ nullptr };

};*/