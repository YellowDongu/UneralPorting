#include "SpawnTrigger.h"
#include "SpawnHandler.h"
#include "ObjectFactory.h"
#include "VehicleBase.h"
#include "SelectPanel.h"
#include "TeamManagementSystem.h"
#include "MissionMode.h"
#include "Aircraft.h"

//#define EndCondVisibleTest
bool UTGTKillEndCondition::Update(float deltaTime)
{
#ifdef EndCondVisibleTest
	UE_LOG(LogTemp, Error, TEXT("[UTGTKillEndCondition::Update] Condition Check"));

	if (ActiveList->Num() > 1)
	{
		UE_LOG(LogTemp, Error, TEXT("[UTGTKillEndCondition::Update] Condition 1 Achived"));
	}
	else
	{
		for (auto& item : *ActiveList)
			UE_LOG(LogTemp, Error, TEXT("[UTGTKillEndCondition::Update] Condition 1 Left In ActiveList %s"), *item->GetActorLabel());
	}


	if (StandbyList->IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[UTGTKillEndCondition::Update] Condition 2 Achived"));
	}
	else
	{
		for (auto& item : *StandbyList)
			UE_LOG(LogTemp, Error, TEXT("[UTGTKillEndCondition::Update] Condition 2 Left In StandbyList %s"), *item->GetActorLabel());
	}
#endif
	
	if(ActiveList->Num() <= 1 || !StandbyList->IsEmpty())
		return false;

	
	if (!Factory->IsTGTEmpty())
		return false;
#ifdef EndCondVisibleTest
	UE_LOG(LogTemp, Error, TEXT("[UTGTKillEndCondition::Update] Condition 3 Achived"));
#endif
	AMissionMode* gameMode = Cast<AMissionMode>(Factory->GetWorld()->GetAuthGameMode());
	check(gameMode);

	gameMode->TriggerWinLocal();
#ifdef EndCondVisibleTest
	UE_LOG(LogTemp, Error, TEXT("[UTGTKillEndCondition::Update] All Condition Achived"));
#endif
	return true;
}

void UTGTKillEndCondition::Initialize(UWorld* world, const TArray<TWeakObjectPtr<class ASpawnTrigger>>* activeList, const TArray<TWeakObjectPtr<class ASpawnTrigger>>* standbyList)
{
	Factory = world->GetSubsystem<UObjectFactory>();
	ActiveList = activeList;
	StandbyList = standbyList;
}

bool UTGTCondition::Update(float deltaTime) { if (Factory->IsTGTEmpty()) UE_LOG(LogTemp, Error, TEXT("[UTGTCondition::Update] Condition Achived, %d"), Factory->GetTGTCount()); return Factory->IsTGTEmpty(); }
void UTGTCondition::Initialize(UWorld* world)
{
	Factory = world->GetSubsystem<UObjectFactory>();
}

ASpawnTrigger::ASpawnTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ASpawnTrigger::PostInitializeComponents()
{
	Super::PostInitializeComponents();

#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::EditorPreview)
		return;
#endif

	for (auto& item : list)
		item.Initialize(this);

	ConditionChecker = CreateSpawnCondition();
	USpawnHandler* spawnHandler = GetWorld()->GetSubsystem<USpawnHandler>();

	check(ConditionChecker);
	check(spawnHandler);

	spawnHandler->Enlist(this);
	UnionFind();
}

bool ASpawnTrigger::Update(float deltaTime) { return ConditionChecker->Update(deltaTime); }

void ASpawnTrigger::Active(UWorld* world)
{
	if (bIsActive) return;

	//UE_LOG(LogTemp, Error, TEXT("[ASpawnTrigger::Active] SpawnTriggerActive"));
	UObjectFactory* objectFactory = world->GetSubsystem<UObjectFactory>();
	UTeamManagementSystem* teamSystem = world->GetSubsystem<UTeamManagementSystem>();

	check(objectFactory);
	check(teamSystem);

	TArray<TWeakObjectPtr<AVehicleBase>> instanceList;
	instanceList.SetNum(list.Num());

	for (int i = 0; i < list.Num(); i++)
	{
		if (instanceList[i] != nullptr)
			continue;

		CreateInstance(i, objectFactory, teamSystem, instanceList);
	}
	
	bIsActive = true;
}

TSharedPtr<USpawnCondition> ASpawnTrigger::CreateSpawnCondition()
{
	TSharedPtr<USpawnCondition> newInstnace;
	switch (SpawnCondition)
	{
	case ESpawnConditionType::Immidiate:
	{
		newInstnace = MakeShared<USpawnCondition>();
		//USpawnCondition* condition = StaticCast<USpawnCondition*>(newInstnace.Get());
		break;
	}
	case ESpawnConditionType::Time:
	{
		newInstnace = MakeShared<UTimeCondition>();
		UTimeCondition* condition = StaticCast<UTimeCondition*>(newInstnace.Get());
		condition->SetTime(Timer, GetWorld()->GetTimerManager());
		break;
	}
	case ESpawnConditionType::TGTCleared:
	{
		newInstnace = MakeShared<UTGTCondition>();
		UTGTCondition* condition = StaticCast<UTGTCondition*>(newInstnace.Get());
		condition->Initialize(GetWorld());
		break;
	}
	case ESpawnConditionType::AllCleared:
	{
		newInstnace = MakeShared<UKillCondition>();
		break;
	}
	case ESpawnConditionType::UTGTKillEnd:
	{
		USpawnHandler* spawnHandler = GetWorld()->GetSubsystem<USpawnHandler>();
		newInstnace = MakeShared<UTGTKillEndCondition>();
		UTGTKillEndCondition* condition = StaticCast<UTGTKillEndCondition*>(newInstnace.Get());
		condition->Initialize(GetWorld(), &spawnHandler->GetActiveList(), &spawnHandler->GetStandbyList());
		break;
	}
	default:
		newInstnace = MakeShared<USpawnCondition>();
		UE_LOG(LogTemp, Error, TEXT("[ASpawnTrigger::CreateSpawnCondition] Condition Type Error"));
		break;
	}

	bActiveReady = true;
	return newInstnace;
}

void ASpawnTrigger::CreateInstance(int index, UObjectFactory* objectFactory, UTeamManagementSystem* teamSystem, TArray<TWeakObjectPtr<AVehicleBase>>& instanceList)
{
	static int32 AntiRecursionCounter = 0;

	if (AntiRecursionCounter >= 10)
	{
		UE_LOG(LogTemp, Error, TEXT("[ASpawnTrigger::CreateInstance] AntiRecursionCounter OverFlow"));
		// need? add : end process
		return;
	}

	AntiRecursionCounter++;
	auto& item = list[index];

	TWeakObjectPtr<AVehicleBase> leader{nullptr};
	TWeakObjectPtr<AVehicleBase>& newInstance = instanceList[index];

	if (item.LeaderIndex != -1)
	{
		if (instanceList[item.LeaderIndex] == nullptr)
			CreateInstance(item.LeaderIndex, objectFactory, teamSystem, instanceList);
		leader = instanceList[item.LeaderIndex];
	}

	newInstance = objectFactory->Instantiate(item, teamSystem, leader, GetWorld());
	if (!newInstance.IsValid())
		return;

	instanceList[index]->SetActorLocationAndRotation(item.GlobalPosition(), item.GlobalRotation());

	if (newInstance->IsAir())
		StaticCast<AAircraft*>(newInstance.Get())->SetGear(false, true);

	AntiRecursionCounter--;
}

void ASpawnTrigger::UnionFind()
{
	TArray<int> parent;

	parent.Reserve(list.Num());
	for (int i = 0; i < list.Num(); i++)
		parent.Add(list[i].LeaderIndex == -1 ? i : list[i].LeaderIndex);

	for (int i = 0; i < list.Num(); i++)
	{
		if (i == parent[i])
			continue;

		parent[i] = UnionFindParentFind(parent[i], parent);
	}

	for (int i = 0; i < list.Num(); i++)
		list[i].LeaderIndex = i == parent[i] ? -1 : parent[i];
}

int ASpawnTrigger::UnionFindParentFind(int index, TArray<int>& parent)
{
	if (parent[index] == index)
		return index;
	parent[index] = UnionFindParentFind(parent[index], parent);
	return parent[index];
}


void ASpawnTrigger::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);


#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::EditorPreview)
		return;

	for (auto& item : ActiveMesh)
	{
		if (!item.IsValid())
			continue;

		item->DestroyComponent();
	}

	TArray<USceneComponent*> children;
	RootComponent->GetChildrenComponents(true, children);

	for (USceneComponent* Child : children)
		if (IsValid(Child))
			Child->DestroyComponent();

	if (GetWorld()->WorldType == EWorldType::PIE)
		return;

	ActiveMesh.Empty();
	if (PreviewMesh.IsNull())
		return;

	auto mesh = PreviewMesh.LoadSynchronous();
	for (auto& item : list)
	{
		auto previewMesh = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass());
		if (previewMesh == nullptr)
			return;

		AddInstanceComponent(previewMesh);
		previewMesh->SetupAttachment(RootComponent);
		previewMesh->SetSkeletalMesh(mesh);
		previewMesh->SetRelativeLocationAndRotation(item.LocalPosition, FRotator(item.LocalRotation.X, item.LocalRotation.Y, item.LocalRotation.Z));
		previewMesh->RegisterComponent();

		ActiveMesh.Add(previewMesh);
	}
#else
	TArray<USceneComponent*> children;
	RootComponent->GetChildrenComponents(true, children);

	for (USceneComponent* Child : children)
		if (IsValid(Child))
			Child->DestroyComponent();
#endif

}
