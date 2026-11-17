// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPilot.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/AudioComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraShakeBase.h"
//#include "../Plugins/Cameras/GameplayCameras/Source/GameplayCameras/Public/WaveOscillatorCameraShakePattern.h"
//#include "../Plugins/Cameras/GameplayCameras/Source/GameplayCameras/Public/LegacyCameraShake.h"
#include "LegacyCameraShake.h"
#include "WaveOscillatorCameraShakePattern.h"

#include "Aircraft.h"
#include "FrontHUD.h"
#include "FireControlSystem.h"
#include "UIHandler.h"
#include "RadarUI.h"
#include "SoundSubsystem.h"
#include "ObjectFactory.h"
#include "Missile.h"


APlayerPilot::APlayerPilot()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
}

void APlayerPilot::BeginPlay(void)
{
    Super::BeginPlay();

    USoundSubsystem* soundSubsystem = GetGameInstance()->GetSubsystem<USoundSubsystem>();
    ShootSound = soundSubsystem->GetSound(TEXT("MissileFired"));

    bool bombSound = false;
    auto specialWeapon = Aircraft->GetFireControlSystem()->GetMissile(true);

    if (specialWeapon.IsValid())
    {
        EWeaponID id = specialWeapon->GetWeaponID();
        bombSound |= id == EWeaponID::UGB;
        bombSound |= id == EWeaponID::LGB;
    }

    SpecialShootSound = soundSubsystem->GetSound(bombSound ? TEXT("BD_00035") : TEXT("MissileFired2"));
    GunSoundSet = MakeShared<FSoundSet>();
    GunSoundSet->Initialize(TEXT("Valcan20mm"), TEXT("GunChannel"), soundSubsystem);
    StaticCast<USoundWave*>(GunSoundSet->Sound)->bLooping = true;

    EngineSoundSet = MakeShared<FSoundSet>();
    EngineSoundSet->Channel = Aircraft->FindComponentByClass<UAudioComponent>();
    EngineSoundSet->ChangeSound(soundSubsystem->GetSound(TEXT("Engine_Outside")));
    StaticCast<USoundWave*>(EngineSoundSet->Sound)->bLooping = true;
    EngineSoundSet->FloatValue = 1.0f;
    EngineSoundSet->Play();
}

void APlayerPilot::SetupInputComponent(void)
{
    Super::SetupInputComponent();

    //if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent)) { Input->BindAction(InteractAction, ETriggerEvent::Started, this, &AMyCharacter::Interact); }
    //FInputKeyBinding& result = InputComponent->BindKey(EKeys::T, IE_Pressed, this, &APlayerPilot::ChangeTarget); => 바인딩을 변경시킬 수 있음
    //InputComponent->KeyBindings.Remove(result);

    InputComponent->BindKey(EKeys::T, IE_Pressed, this, &APlayerPilot::ChangeTarget);
    
    InputComponent->BindKey(EKeys::G, IE_Pressed, this, &APlayerPilot::GearDeploy);
    InputComponent->BindKey(EKeys::E, IE_Pressed, this, &APlayerPilot::ChangeMissile);
    InputComponent->BindKey(EKeys::F, IE_Pressed, this, &APlayerPilot::DeployFlare);
    InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &APlayerPilot::ShootWeapon);
    //InputComponent->BindKey(EKeys::LeftControl, IE_Pressed, this,&APlayerPilot::ShootGun);
    //InputComponent->BindKey(EKeys::LeftControl, IE_Released, this,&APlayerPilot::ShootGun);

    InputComponent->BindKey(EKeys::NumPadFive, IE_Pressed, this, &APlayerPilot::ResetCamera);

    InputComponent->BindKey(EKeys::NumPadSix, IE_Pressed, this, &APlayerPilot::YawCamRotatePlus);
    InputComponent->BindKey(EKeys::NumPadSix, IE_Released, this, &APlayerPilot::YawCamRotateMinus);
    InputComponent->BindKey(EKeys::NumPadFour, IE_Released, this, &APlayerPilot::YawCamRotatePlus);
    InputComponent->BindKey(EKeys::NumPadFour, IE_Pressed, this, &APlayerPilot::YawCamRotateMinus);

    InputComponent->BindKey(EKeys::NumPadTwo, IE_Pressed, this, &APlayerPilot::PitchCamRotatePlus);
    InputComponent->BindKey(EKeys::NumPadTwo, IE_Released, this, &APlayerPilot::PitchCamRotateMinus);
    InputComponent->BindKey(EKeys::NumPadEight, IE_Released, this, &APlayerPilot::PitchCamRotatePlus);
    InputComponent->BindKey(EKeys::NumPadEight, IE_Pressed, this, &APlayerPilot::PitchCamRotateMinus);

    InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &APlayerPilot::YokeXPlus);
    InputComponent->BindKey(EKeys::Right, IE_Released, this, &APlayerPilot::YokeXMinus);
    InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &APlayerPilot::YokeXMinus);
    InputComponent->BindKey(EKeys::Left, IE_Released, this, &APlayerPilot::YokeXPlus);

    InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &APlayerPilot::YokeZMinus);
    InputComponent->BindKey(EKeys::Up, IE_Released, this, &APlayerPilot::YokeZPlus);
    InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &APlayerPilot::YokeZPlus);
    InputComponent->BindKey(EKeys::Down, IE_Released, this, &APlayerPilot::YokeZMinus);

    InputComponent->BindKey(EKeys::D, IE_Pressed, this, &APlayerPilot::YokeYPlus);
    InputComponent->BindKey(EKeys::D, IE_Released, this, &APlayerPilot::YokeYMinus);
    InputComponent->BindKey(EKeys::A, IE_Pressed, this, &APlayerPilot::YokeYMinus);
    InputComponent->BindKey(EKeys::A, IE_Released, this, &APlayerPilot::YokeYPlus);
}

void APlayerPilot::ChangeTarget()
{
    TArray<FTargetInfo>* currentTargets;

    TWeakObjectPtr<UFireControlSystem> fcs = Aircraft->GetFireControlSystem();
    fcs->GetCurrentTargets(currentTargets);

    for (auto& item : *currentTargets)
        SetSelectTargetUI.Broadcast(item.Target.Get(), false);

    fcs->ChangeTarget();

    if (fcs->GetSelectSpecial())
        for (auto& item : *currentTargets)
            SetSelectTargetUI.Broadcast(item.Target.Get(), true);
    else if(!currentTargets->IsEmpty())
        SetSelectTargetUI.Broadcast((*currentTargets)[0].Target.Get(), true);
}

void APlayerPilot::GearDeploy(void) { Aircraft->DeployGear(); }
void APlayerPilot::ChangeMissile(void) { ChangeWeaponUI.Execute(Aircraft->ChangeMissile()); }
void APlayerPilot::DeployFlare(void) { Aircraft->StartDeployFlare(); }

void APlayerPilot::ShootWeapon(void)
{
    if (Aircraft->ShootMissile(Container))
    {
        bool isSpecial = Aircraft->GetSelectSpecial();
        UGameplayStatics::PlaySound2D(this, !isSpecial ? ShootSound : SpecialShootSound);
        FiredMissile.Execute(Container, isSpecial);
    }
}

void APlayerPilot::ShootGun(void)
{
    if (bShootGun != Super::IsInputKeyDown(EKeys::LeftControl))
    {
        bShootGun = !bShootGun;

        if (bShootGun)
            GunSoundSet->Play();
        else
            GunSoundSet->Stop();
    }
}

void APlayerPilot::ResetCamera(void) { ThirdCameraArm->SetRelativeRotation(FQuat::Identity); }

#include "AIAircraftState.h"
TSharedPtr<ManeuverState> testmodule;
void StartTest(APlayerPilot* pilot, AAircraft* aircraft)
{
    if (testmodule.IsValid())
    {
        if (testmodule->AIAircraftState::Start())
            return;

        testmodule->Release();
        testmodule.Reset();
    }

    AIAircraftState::AircraftDefaultData defaultData;
    aircraft->GetControl(defaultData.Control);
    defaultData.Owner = aircraft;
    defaultData.SquadData = &aircraft->GetSquadData();

    TSharedPtr<ManeuverState::ManeuverData> data = MakeShared<ManeuverState::ManeuverData>();
    data->ManeuverName = TEXT("ImmelmannTurnRight");

    testmodule = MakeShared<ManeuverState>();
    testmodule->SetDefaultData(&defaultData);
    testmodule->AddAviliableManeuver(data->ManeuverName);
    //testmodule->AddAviliableManeuver(TEXT("ImmelmannTurnLeft"));
    if (!testmodule->Start(data))
    {
        testmodule->Release();
        testmodule.Reset();
    }
    else
        UE_LOG(LogTemp, Log, TEXT("[::StartTest] StartTest"));
}

bool TestModuleUpdate()
{
    if (!testmodule.IsValid())
        return false;

    if (testmodule->Update())
    {
        testmodule->Release();
        testmodule.Reset();
    }

    return true;
}

void APlayerPilot::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    bool w = Super::IsInputKeyDown(EKeys::W);
    bool s = Super::IsInputKeyDown(EKeys::S);

    //static constexpr float MinValue = 1.0f;
    //static constexpr float MaxValue = 2.5f;

    float nextPitch = (Control->Throttle + 0.8f)/* * 1.1f*/;
    if (FMath::Abs(nextPitch - EngineSoundSet->FloatValue) > 0.01f)
    {
        EngineSoundSet->FloatValue = FMath::FInterpTo(EngineSoundSet->FloatValue, nextPitch, DeltaTime, 1.0f);
        EngineSoundSet->Channel->SetPitchMultiplier(EngineSoundSet->FloatValue);
    }

    if (w)
    {
        Control->isAirBrakeOn = false;
        if (s)
        {
            Control->Throttle = 0.2f;
            Control->bActiveHighGTurn = true;
        }
        else
        {
            Control->Throttle = 1.0f;
            Control->bActiveHighGTurn = false;
        }
    }
    else
    {
        Control->bActiveHighGTurn = false;
        if (s)
        {
            Control->Throttle = 0.1f;
            Control->isAirBrakeOn = true;
        }
        else
        {
            Control->Throttle = 0.2f;
            Control->isAirBrakeOn = false;
        }
    }

    if (bShootGun != Super::IsInputKeyDown(EKeys::LeftControl))
    {
        bShootGun = !bShootGun;

        if (bShootGun)
            GunSoundSet->Play();
        else
            GunSoundSet->Stop();
    }

    if (bShootGun)
        Aircraft->GetFireControlSystem()->ShootGun();

    //if (TestModuleUpdate())
    //    return;
    //
    //if (Super::IsInputKeyDown(EKeys::F4))
    //    StartTest(this, Aircraft.Get());

    //if (Super::WasInputKeyJustPressed(EKeys::R))
    //    attachedAircraft->DeployGear();
    if (Super::WasInputKeyJustPressed(EKeys::P))
        CheatDeleteEnemy(GetWorld());

    if (Super::WasInputKeyJustPressed(EKeys::O))
        CameraExplosionShake(1.0f, 0.4f, 2.0f);
    ThirdCameraArm->AddLocalRotation(FRotator(PitchCamRotate * CamRotateSpeed, YawCamRotate * CamRotateSpeed, 0.0f));

#if !UE_BUILD_SHIPPING
    return;
    Control->Yoke.X = static_cast<double>(Super::IsInputKeyDown(EKeys::Right)) - static_cast<double>(Super::IsInputKeyDown(EKeys::Left));
    Control->Yoke.Z = static_cast<double>(Super::IsInputKeyDown(EKeys::Down)) - static_cast<double>(Super::IsInputKeyDown(EKeys::Up));
    Control->Yoke.Y = static_cast<double>(Super::IsInputKeyDown(EKeys::D)) - static_cast<double>(Super::IsInputKeyDown(EKeys::A));

    if (Super::IsInputKeyDown(EKeys::NumPadFive))
        ThirdCameraArm->SetRelativeRotation(FQuat::Identity);
    else
    {
        double camRotateSpeed = 5.0f;
        //float yawCamRotate = static_cast<double>(Super::IsInputKeyDown(EKeys::NumPadFour)) - static_cast<double>(Super::IsInputKeyDown(EKeys::NumPadSix));
        //float pitchCamRotate = static_cast<double>(Super::IsInputKeyDown(EKeys::NumPadEight)) - static_cast<double>(Super::IsInputKeyDown(EKeys::NumPadTwo));
        double yawCamRotate = static_cast<double>(Super::IsInputKeyDown(EKeys::NumPadSix)) - static_cast<double>(Super::IsInputKeyDown(EKeys::NumPadFour));
        double pitchCamRotate = static_cast<double>(Super::IsInputKeyDown(EKeys::NumPadTwo)) - static_cast<double>(Super::IsInputKeyDown(EKeys::NumPadEight));
        ThirdCameraArm->AddLocalRotation(FRotator(pitchCamRotate * camRotateSpeed, yawCamRotate * camRotateSpeed, 0.0f));
    }

    if (Super::WasInputKeyJustPressed(EKeys::T))
    {
        TArray<FTargetInfo>* currentTargets;
        TWeakObjectPtr<UFireControlSystem> fcs = Aircraft->GetFireControlSystem();

        fcs->GetCurrentTargets(currentTargets);

        for (auto& item : *currentTargets)
            SetSelectTargetUI.Broadcast(item.Target.Get(), false);

        fcs->ChangeTarget();

        for (auto& item : *currentTargets)
            SetSelectTargetUI.Broadcast(item.Target.Get(), true);
    }


    if (Super::WasInputKeyJustPressed(EKeys::G))
        Aircraft->DeployGear();

    if (Super::WasInputKeyJustPressed(EKeys::E))
        ChangeWeaponUI.Execute(Aircraft->ChangeMissile());

    if (Super::WasInputKeyJustPressed(EKeys::F))
        Aircraft->StartDeployFlare();

    if (Super::WasInputKeyJustPressed(EKeys::SpaceBar))
        if (Aircraft->ShootMissile(Container))
        {
            FiredMissile.Execute(Container, Aircraft->GetSelectSpecial());

        }

    if (Super::IsInputKeyDown(EKeys::LeftControl))
        Aircraft->GetFireControlSystem()->ShootGun();
    //if (attachedAircraft->GetFireControlSystem()->ShootGun())
    //    FiredMissile.Execute(Container, attachedAircraft->GetSelectSpecial());
#endif
}

void APlayerPilot::Attach(AAircraft* aircraft)
{
    check(aircraft);
    Super::Possess(aircraft);
    aircraft->GetControl(Control);
    aircraft->AttachCamera(PlayerCamera);
    SetViewTarget(aircraft);

    FirstCameraArm = aircraft->GetFirstCameraArm();
    ThirdCameraArm = aircraft->GetThirdCameraArm();
    
	Aircraft = aircraft;
    LinkHUD();

    PrimaryActorTick.bStartWithTickEnabled = true;
    SetActorTickEnabled(true);
    Container.Reserve(aircraft->GetFireControlSystem()->GetMultiShoot());
    aircraft->SetGear(true, true);

    aircraft->AddDamageEvent(AVehicleBase::FOnDamageEvent::FDelegate::CreateWeakLambda(this, [this](AVehicleBase* target)-> void { CameraExplosionShake(0.5f, 0.4f, 1.0f); }));
}

void APlayerPilot::Detach(void)
{
    //if (Aircraft.IsValid())
    //{
    //    Aircraft->DetachCamera(PlayerCamera);
    //    SetViewTarget(nullptr);
    //}

    Super::UnPossess();
    Aircraft.Reset();
    PrimaryActorTick.bStartWithTickEnabled = false;
	SetActorTickEnabled(false);
}

void APlayerPilot::LinkHUD(void)
{
    if (FrontHUD == nullptr)
    {
        AUIHandler* hud = Cast<AUIHandler>(MyHUD);
        check(hud);
        FrontHUD = hud->CreateFrontHUD();
        check(FrontHUD.Get());
    }

    FrontHUD->BindVehicle(Aircraft.Get(), this);
    //InputComponent->KeyBindings.RemoveAll([this](const FInputKeyBinding& binding) -> bool { return binding.Chord.Key == EKeys::R && binding.KeyEvent == IE_Released; });
    InputComponent->KeyBindings.RemoveAll([this](const FInputKeyBinding& binding) -> bool { return binding.Chord.Key == EKeys::R; });
    InputComponent->BindKey(EKeys::R, IE_Released, FrontHUD->GetRadarUI(), &URadarUI::ChangeRadar);
}


//#include "../Plugins/Cameras/GameplayCameras/Source/GameplayCameras/Public/WaveOscillatorCameraShakePattern.h"
//#include "../Plugins/Cameras/GameplayCameras/Source/GameplayCameras/Public/LegacyCameraShake.h"
void APlayerPilot::CameraExplosionShake(float duration, float attenuation, float strength)
{
    ULegacyCameraShake* Shake = StaticCast<ULegacyCameraShake*>(PlayerCameraManager->StartCameraShake(ULegacyCameraShake::StaticClass(), 1.0f));

    // 지속 시간
    Shake->OscillationDuration = duration;

    // 현재 재생 중인 진동의 남은 시간도 설정
    Shake->OscillatorTimeRemaining = Shake->OscillationDuration;

    // 감쇠 시간
    Shake->OscillationBlendInTime = 0.1f;
    Shake->OscillationBlendOutTime = attenuation;

    // Pitch 진폭 및 주파수
    Shake->RotOscillation.Pitch.Amplitude = strength;
    Shake->RotOscillation.Pitch.Frequency = 20.0f;

    // Yaw 진폭 및 주파수
    Shake->RotOscillation.Yaw.Amplitude = strength;
    Shake->RotOscillation.Yaw.Frequency = 15.0f;

    //check(Shake)
    //UWaveOscillatorCameraShakePattern* Pattern = Shake->ChangeRootShakePattern<UWaveOscillatorCameraShakePattern>();
    //if (Pattern)
    //{
    //    Pattern->Duration = 0.5f;
    //    Pattern->BlendInTime = 0.1f;
    //    Pattern->BlendOutTime = 0.4f;
    //    // 진폭과 주파수도 여기서 설정
    //}
}