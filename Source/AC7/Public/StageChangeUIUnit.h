// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageChangeUIUnit.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UStageChangeUIUnit : public UUserWidget
{
	GENERATED_BODY()
public:
	DECLARE_MULTICAST_DELEGATE(FCallBackFunction);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	void EnterMission();
	void ReturnMainMenu(bool EndMission = false);
	void ExitSelect(const FCallBackFunction::FDelegate& CallBackDelegate);
	void EnterSelect(const FCallBackFunction::FDelegate& CallBack);
	//void ExitSelect(const TArray<FCallBackFunction::FDelegate>& CallBackDelegate);
	//void EnterSelect(const TArray<FCallBackFunction::FDelegate>& CallBack);
	void FadeOut(bool active, float fadeoutTime);
	void FadeOut(bool active);
	void FadeOutCoroutine();
	void FadeInCoroutine();

	bool IsFadingOut() { return FadeOutTimerHandle.IsValid(); }
	bool IsFadeOut() { return bIsFadeOut; }

	FDelegateHandle AddCallBack(const FCallBackFunction::FDelegate& Delegate) { return CallBack.Add(Delegate); }
	void RemoveCallBack(FDelegateHandle Handle) { CallBack.Remove(Handle); }

private:
	bool StartSelectLevel(int& phase, float& phaseTime);
	bool InterMission(int& phase, float& phaseTime, float deltaTime);
	bool InterSelect(int& phase, float& phaseTime);
	bool ExitSelect(int& phase, float& phaseTime, float deltaTime);
	void GetLoadingEndFlag();
	bool ReturnMenu(int& phase, float& phaseTime, float deltaTime);
	void ActiveCallBack(const FString& name) { auto callback = CallBacks.Find(name); if (callback != nullptr) { callback->Broadcast(); CallBacks.Remove(name); } }

	float GetDeltaTime();

	bool bIsFadeOut{ true };
	bool bLoadingEndFlag{ false };
	float FadeOutTime{ 1.0f }, FadeOutTimer{ 0.0f };

	FTimerHandle FadeOutTimerHandle; // 핸들을 하나로 합쳐야됨
	FTimerHandle InterSelectHandle;
	FCallBackFunction CallBack;
	TMap<FString, FCallBackFunction> CallBacks;
	TArray<TFunction<bool(float)>> TickFunctions;
	TSet<FString> ActivedFunction;

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UImage* BlackFadeOut{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCanvasPanel* LoadingCanvas{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCanvasPanel* MissionPanel{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* MissionText{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCanvasPanel* MissionLoadingPanel{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCanvasPanel* MissionSuccessPanel{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* MainText{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* SubText{ nullptr };
};
