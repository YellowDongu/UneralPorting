// Fill out your copyright notice in the Description page of Project Settings.


#include "Radar.h"
//#include "Components/BoxComponent.h"
//#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameMaster.h"
#include "BluePrintCollector.h"
#include "Aircraft.h"
#include "ObjectFactory.h"
#include "Missile.h"
#include "Flare.h"
#include "IFFHandler.h"
#include "SoundSubsystem.h"

URadar::URadar()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URadar::BeginPlay()
{
	Super::BeginPlay();

	for (USceneComponent* Child : GetOwner()->GetRootComponent()->GetAttachChildren())
		if (Child->GetFName().ToString().Contains(TEXT("FlarePod")))
			flarePod.Add(Child);
}

void URadar::SystemIntegration(TWeakObjectPtr<class USphereComponent> collider, int _team, float _raderDistance)
{
	if (collider == nullptr)
		return;

	AAircraft* Owner = Cast<AAircraft>(GetOwner());
	if (Owner != nullptr)
		Owner->GetControl(Control);

	Collider = collider;
	Team = _team;
	RadarRange = _raderDistance;
	FlareRPM = 1.0f / (FlareRPM / 60.0f);

	//layerMask = LayerMask.GetMask("MovingVehicle");
	//targetLayer = LayerMask.NameToLayer("MovingVehicle");
	//raderDistance = GameMaster.ConvertWorldScale(raderDistance);
	//TSet<AActor*> OverlappingActors;
	//Collider->GetOverlappingActors(OverlappingActors, AVehicleBase::StaticClass());
	//
	//for (auto& item : OverlappingActors)
	//{
	//AVehicleBase* vehicle = Cast<AVehicleBase>(item);
	//
	//if (vehicle != nullptr)
	//{
	//	if (GetOwner() == vehicle)
	//		return;
	//	if (static_cast<AVehicleBase*>(GetOwner())->GetTeam() == vehicle->GetTeam()) return;
	//
	//	bool isAleadyIn;
	//	InDistance.Add(vehicle, &isAleadyIn);
	//	if (!isAleadyIn)
	//		AddTarget.Broadcast(vehicle);
	//
	//}
	//}

	Collider->OnComponentBeginOverlap.AddDynamic(this, &URadar::OnTriggerEnter);
	Collider->OnComponentEndOverlap.AddDynamic(this, &URadar::OnTriggerExit);
	//Collider->OnComponentHit.AddDynamic(this, &URadar::OnHit);
	Collider->SetSphereRadius(RadarRange);

	Audio = Cast<AVehicleBase>(GetOwner())->FindComponentByClass<UAudioComponent>();
	FlareFire = GetWorld()->GetGameInstance()->GetSubsystem<USoundSubsystem>()->GetSound(TEXT("FlareFire"));
}

void URadar::BindUI(UIFFHandler* handler)
{
	Collider->OnComponentBeginOverlap.AddDynamic(handler, &UIFFHandler::OnTriggerEnter);
	Collider->OnComponentEndOverlap.AddDynamic(handler, &UIFFHandler::OnTriggerExit);
}

void URadar::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UPrimitiveComponent* Comp = OtherComp;
	AActor* Actor = OtherActor;

	AVehicleBase* vehicle = Cast<AVehicleBase>(OtherActor);

	if (vehicle != nullptr)
	{
		if (GetOwner() == vehicle)
			return;

		if (!InRange.Contains(vehicle))
			InRange.Add(vehicle);
		if (static_cast<AVehicleBase*>(GetOwner())->GetTeam() == vehicle->GetTeam()) return;

		bool isAleadyIn;
		InDistance.Add(vehicle, &isAleadyIn);
		if (!isAleadyIn)
		{
			AddTarget.Broadcast(vehicle);
			UE_LOG(LogTemp, Error, TEXT("[HoldState::Update] Spotted %s -> %s"), *GetOwner()->GetName(), *OtherActor->GetName());
		}
	}
}

void URadar::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AVehicleBase* vehicle = Cast<AVehicleBase>(OtherActor);

	InRange.Remove(vehicle);

	if (vehicle != nullptr && InDistance.Contains(vehicle))
	{
		InDistance.Remove(vehicle);
		RemoveTarget.Broadcast(vehicle);
	}
}

void URadar::DeployFlare()
{
	if (flarePod.IsEmpty())
		return;

	DeployAmount--;

	int TargetNum = Tracing.Num();
	if (TargetNum > 4) TargetNum /= 2;
	int SubTargetNum = TargetNum / flarePod.Num();
	int length = 0, i = 0;

	UObjectFactory* objectFactory = GetWorld()->GetSubsystem<UObjectFactory>();
	check(objectFactory);
	for (auto& item : flarePod)
	{
		TWeakObjectPtr<AFlare> newInstance = objectFactory->GetFlare(item.Get());
		length = FMath::Min(i + SubTargetNum, flarePod.Num());
		for (auto& missile : Tracing)
		{
			if (i++ >= length) break;
	
			missile->ForceChangeTarget(newInstance);
		}
	}
	//UGameplayStatics::PlaySound2D(this, FlareFire);
	UGameplayStatics::PlaySoundAtLocation(this, FlareFire, GetOwner()->GetActorLocation());

	if (DeployAmount < 0)
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void URadar::StartDeployFlare()
{
	if (TimerHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[URadar::StartDeployFlare] Flare Deploying : %s"), *GetOwner()->GetName());
	}
	else if (FlareCount <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[URadar::StartDeployFlare] Flare CoolTime : %s"), *GetOwner()->GetName());
	}


	if (TimerHandle.IsValid() || FlareCount <= 0)
		return;

	FTimerHandle dummy;
	FlareCount--;
	DeployAmount = FlareDeployCount;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &URadar::DeployFlare, FlareRPM, true);
	GetWorld()->GetTimerManager().SetTimer(dummy, [this]() -> void { FlareCount++; }, FlareCoolTime, false);
}

void URadar::Remove(TArray<AVehicleBase*>& targets)
{
	int length = targets.Num();
	for (int i = 0; i < length; i++)
		InDistance.Remove(targets[i]);
}

void URadar::CallWarning(double squareDistance)
{
	static constexpr double alartDistance{ ConvertToDistance(1000.0) * ConvertToDistance(1000.0) };

	if (squareDistance <= alartDistance)
		bAlart = true;

	UE_LOG(LogTemp, Warning, TEXT("[URadar::CallWarning] %s to %s"), bAlart ? TEXT("Alart") : TEXT("Warning"), *GetOwner()->GetName());
	bWarning = true;
}

//public void DeployFlare(AudioSource source, AudioClip sound)
//{
//    if (Deploying)
//        return;
//
//    if (FlareCount <= 0)
//        return;
//
//    StartCoroutine(Deploy(source, sound));
//    StartCoroutine(FlareCoolTime());
//}
//
//public IEnumerator Deploy(AudioSource source, AudioClip sound)
//{
//    Deploying = true;
//    int Count = deployCount;
//    float Timer = 0;
//    while (Count > 0)
//    {
//        Timer += Time.deltaTime;
//        if (Timer < rpm)
//        {
//            yield return null;
//            continue;
//        }
//        Timer = 0.0f;
//        Count--;
//
//        if (Tracing.Count > 0)
//        {
//            int count = (Tracing.Count + flarePod.Length - 1) / flarePod.Length; // ºü¸¥ ³ª´°¼À ¿Ã¸²
//            int i = 0, length;
//            Missile[] collection = Tracing.ToArray();
//            Tracing.Clear();
//
//            foreach(var item in flarePod)
//            {
//                Flare newInstance = GameMaster.Instance.Factory.ShootFlare(item);
//                length = Mathf.Clamp(i + count, 0, collection.Length);
//                for (; i < length; i++)
//                {
//                    collection[i].ChangeTarget(newInstance.Distruption());
//                    collection[i].MissileWarning -= Warning;
//                }
//                i += count;
//            }
//            source.PlayOneShot(sound);// GameMaster.GetInstance().Sound().PlayOnce(source, sound);
//        }
//        else
//        {
//            foreach(var item in flarePod)
//                GameMaster.Instance.Factory.ShootFlare(item);
//            source.PlayOneShot(sound);// GameMaster.GetInstance().Sound().PlayOnce(source, sound);
//        }
//
//        yield return null;
//    }
//    Deploying = false;
//}
//
//public IEnumerator Deploy()
//{
//    Deploying = true;
//    int Count = deployCount;
//    float Timer = 0;
//    while (Count > 0)
//    {
//        Timer += Time.deltaTime;
//        if (Timer < rpm)
//        {
//            yield return null;
//            continue;
//        }
//        Timer = 0.0f;
//        Count--;
//
//        if (Tracing.Count > 0)
//        {
//            int count = (Tracing.Count + flarePod.Length - 1) / flarePod.Length; // ºü¸¥ ³ª´°¼À ¿Ã¸²
//            int i = 0, length;
//            Missile[] collection = Tracing.ToArray();
//            Tracing.Clear();
//
//            foreach(var item in flarePod)
//            {
//                Flare newInstance = GameMaster.Instance.Factory.ShootFlare(item);
//                length = Mathf.Clamp(i + count, 0, collection.Length);
//                for (; i < length; i++)
//                {
//                    collection[i].ChangeTarget(newInstance.Distruption());
//                    collection[i].MissileWarning -= Warning;
//                }
//                i += count;
//            }
//        }
//        else
//            foreach(var item in flarePod)
//            GameMaster.Instance.Factory.ShootFlare(item);
//
//        yield return null;
//    }
//    Deploying = false;
//}
//
//public void AutoDeployFlare()
//{
//    if (!AutoFlare || Deploying)
//        return;
//
//    if (FlareCount <= 0)
//        return;
//
//    if (Tracing.Count >= 1)
//    {
//        StartCoroutine(Deploy());
//        StartCoroutine(FlareCoolTime());
//    }
//}
//
//public IEnumerator FlareCoolTime(Action<float> coolTimeMethod = null)
//{
//    FlareCount--;
//    float timer = flareCoolTime;
//
//    if (coolTimeMethod == null)
//        while (timer > 0)
//        {
//            timer -= Time.deltaTime;
//            yield return null;
//        }
//    else
//        while (timer > 0)
//        {
//            timer -= Time.deltaTime;
//            coolTimeMethod(timer);
//            yield return null;
//        }
//
//    FlareCount++;
//}
