#include "Core/EDGameDataSubsystem.h"

#include "Core/EDAssetManager.h"
#include "Kismet/GameplayStatics.h"

const FPrimaryAssetType UEDGameDataSubsystem::LobbyAssetType = FPrimaryAssetType(TEXT("LobbyData"));
const FPrimaryAssetType UEDGameDataSubsystem::ItemAssetType = FPrimaryAssetType(TEXT("ItemData"));
const FPrimaryAssetType UEDGameDataSubsystem::MonsterAssetType = FPrimaryAssetType(TEXT("MonsterData"));

void UEDGameDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadPhase_Lobby();
}

void UEDGameDataSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UEDGameDataSubsystem* UEDGameDataSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GI) return nullptr;

	return GI->GetSubsystem<UEDGameDataSubsystem>();
}

// 외부에서 호출
void UEDGameDataSubsystem::InitializeGameData()
{
	if (CurrentPhase != EDataLoadPhase::LobbyReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EDGameDataSubsystem - InitializeGameData] 로비 데이터가 아직 준비되지 않았습니다."));
		return;
	}
	UnloadPhaseData(LobbyAssetType, TEXT("Lobby"));
	LoadPhase_Item();
}

// ================================================================
// 언로드
// ================================================================

void UEDGameDataSubsystem::UnloadPhaseData(const FPrimaryAssetType& AssetType, const FName& HandleKey)
{
	UEDAssetManager& AM = UEDAssetManager::Get();
	
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(AssetType, Ids);
	
	for (const FPrimaryAssetId& Id : Ids)
	{
		DataCache.Remove(Id);
	}
	
	if (TSharedPtr<FStreamableHandle>* FoundHandle = PhaseHandles.Find(HandleKey))
	{
		if (FoundHandle->IsValid())
		{
			(*FoundHandle)->ReleaseHandle();
		}
		PhaseHandles.Remove(HandleKey);
	}

	if (!Ids.IsEmpty())
	{
		AM.UnloadPrimaryAssets(Ids);
	}
}

// ================================================================
// 단계별 로드 구현
// ================================================================

void UEDGameDataSubsystem::LoadPhase_Lobby()
{
	SetPhase(EDataLoadPhase::LoadingLobby);
	UEDAssetManager& AM = UEDAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(LobbyAssetType, Ids);
	if (Ids.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EDGameDataSubsystem - LoadPhase_Lobby] Data가 없습니다. 다음 단계로 건너뜁니다."));
		OnLobbyDataLoaded();
		return;
	}
	
	TSharedPtr<FStreamableHandle> Handle = AM.LoadPrimaryAssetsAsync(
		Ids,
		{TEXT("Lobby")},
		FStreamableDelegate::CreateUObject(this, &UEDGameDataSubsystem::OnLobbyDataLoaded)
	);
	
	PhaseHandles.Add(TEXT("Lobby"), Handle);
}

void UEDGameDataSubsystem::LoadPhase_Item()
{
	SetPhase(EDataLoadPhase::LoadingItem);
	UEDAssetManager& AM = UEDAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(ItemAssetType, Ids);
	if (Ids.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EDGameDataSubsystem - LoadPhase_Item] Data가 없습니다. 다음 단계로 건너뜁니다."));
		OnItemDataLoaded();
		return;
	}
	
	TSharedPtr<FStreamableHandle> Handle = AM.LoadPrimaryAssetsAsync(
		Ids,
		{TEXT("ITEM")},
		FStreamableDelegate::CreateUObject(this, &UEDGameDataSubsystem::OnItemDataLoaded)
	);
	PhaseHandles.Add(TEXT("Item"), Handle);
}

void UEDGameDataSubsystem::LoadPhase_Monster()
{
	SetPhase(EDataLoadPhase::LoadingMonster);
	UEDAssetManager& AM = UEDAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(MonsterAssetType, Ids);
	if (Ids.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EDGameDataSubsystem - LoadPhase_Monster] Data가 없습니다. 다음 단계로 건너뜁니다."));
		OnMonsterDataLoaded();
		return;
	}
	
	TSharedPtr<FStreamableHandle> Handle = AM.LoadPrimaryAssetsAsync(
		Ids,
		{TEXT("Monster")},
		FStreamableDelegate::CreateUObject(this, &UEDGameDataSubsystem::OnMonsterDataLoaded)
	);
	PhaseHandles.Add(TEXT("Monster"), Handle);
}

// ================================================================
// 단계별 완료 콜백 함수
// ================================================================

/**
 * 로비에서 필요한 데이터 다 로드하고 나서 오는 곳
 * 여기서는 다음 페이즈로 넘기지 않고, 외부에서 (게임씬같은곳에서 게임 화면으로 넘어갈때) 
 * InitializeGameData를 실행해서 다음 로드를 시작
 */
void UEDGameDataSubsystem::OnLobbyDataLoaded()
{
	CacheLoadedAssets(LobbyAssetType);
	SetPhase(EDataLoadPhase::LobbyReady);
	OnLobbyDataReady.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("[EDGameDataSubsystem - OnLobbyDataLoaded] Lobby 데이터 로드 완료"));
}

void UEDGameDataSubsystem::OnItemDataLoaded()
{
	CacheLoadedAssets(ItemAssetType);
	UE_LOG(LogTemp, Log, TEXT("[EDGameDataSubsystem - OnItemDataLoaded] Item 데이터 로드 완료"));
	LoadPhase_Monster();
}

void UEDGameDataSubsystem::OnMonsterDataLoaded()
{
	CacheLoadedAssets(MonsterAssetType);
	UE_LOG(LogTemp, Log, TEXT("[EDGameDataSubsystem - OnMonsterDataLoaded]  Monster 데이터 로드 완료"));
	SetPhase(EDataLoadPhase::Completed);
	
	// 완료 신호
	OnAllDataLoaded.Broadcast();
}

// ================================================================
// 캐싱 함수
// ================================================================

void UEDGameDataSubsystem::CacheLoadedAssets(const FPrimaryAssetType& AssetType)
{
	UEDAssetManager& AM = UEDAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(AssetType, Ids);
	for (const FPrimaryAssetId& Id : Ids)
	{
		UObject* Obj = AM.GetPrimaryAssetObject(Id);
		if (!Obj)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EDGameDataSubsystem - CacheLoadedAssets] 로드 실패 : %s"), *Id.ToString());
			continue;
		}
		DataCache.Add(Id, Obj);
	}
}

// ================================================================
// 상태 전이
// ================================================================

void UEDGameDataSubsystem::SetPhase(EDataLoadPhase NewPhase)
{
	CurrentPhase = NewPhase;
	OnPhaseChanged.Broadcast(NewPhase);
}