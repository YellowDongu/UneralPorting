// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UIHandler.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API AUIHandler : public AHUD
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void InitializeTitle();
	UFUNCTION(BlueprintCallable)
	void InitializeSelectUI(USelectPanel*& output);
	virtual void PostInitializeComponents(void) override;
	virtual void BeginPlay(void) override;

	inline TWeakObjectPtr<class UMainMenuUI> CreateMainMenuUI() { CreateUI(MainMenuUIBlueprint, MainMenuUI); return MainMenuUI; }
	inline TWeakObjectPtr<class UFrontHUD> CreateFrontHUD() { CreateUI(FrontHUDBlueprint, FrontHUD); return FrontHUD; }
	inline TWeakObjectPtr<class USelectPanel> CreateSelectUI() { CreateUI(SelectPanelBlueprint, SelectPanel); return SelectPanel; }
	inline TWeakObjectPtr<class UStageChangeUIUnit> CreateStageChangeUIUnit() { CreateUI(StageChangeUIBlueprint, StageChangeUIUnit, 100); return StageChangeUIUnit; }
	inline TWeakObjectPtr<class UTitleUI> CreateTitleUI() { CreateUI(TitleUIBlueprint, TitleUI); return TitleUI; }


	void SetFadeOut(bool value);
	void SetFadeOut(bool value, float floatValue);

	inline TWeakObjectPtr<class UMainMenuUI> GetMainMenuUI() { return MainMenuUI; }
	inline TWeakObjectPtr<class UFrontHUD> GetFrontHUD() { return FrontHUD; }
	inline TWeakObjectPtr<class USelectPanel> GetSelectUI() { return SelectPanel; }
	inline TWeakObjectPtr<class UStageChangeUIUnit> GetStageChangeUIUnit() { return StageChangeUIUnit; }
	inline TWeakObjectPtr<class UTitleUI> GetTitleUI() { return TitleUI; }

	static void ResetTrigger(void) { bFirstTime = true; }
private:
	//template<typename Type, typename = std::enable_if_t<std::is_base_of_v<class UUserWidget, Type>>>
	template<typename Type>
	void CreateUI(TSubclassOf<Type>& blueprint, TWeakObjectPtr<Type>& output, int order = 0); // 같은 코드를 여러번 치지 않기 위해, 이 클래스 전용

	TWeakObjectPtr<class UFrontHUD> FrontHUD{ nullptr };
	TWeakObjectPtr<class USelectPanel> SelectPanel{ nullptr };
	TWeakObjectPtr<class UStageChangeUIUnit> StageChangeUIUnit{ nullptr };
	TWeakObjectPtr<class UTitleUI> TitleUI{ nullptr };
	TWeakObjectPtr<class UMainMenuUI> MainMenuUI{ nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UFrontHUD> FrontHUDBlueprint{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class USelectPanel> SelectPanelBlueprint{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UStageChangeUIUnit> StageChangeUIBlueprint{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UTitleUI> TitleUIBlueprint{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UMainMenuUI> MainMenuUIBlueprint{};

	static bool bFirstTime;
};