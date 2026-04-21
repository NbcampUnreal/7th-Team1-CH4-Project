#include "Core/EDGameDataSubsystem.h"

#include "Core/EDAssetManager.h"
#include "Kismet/GameplayStatics.h"

const FPrimaryAssetType UEDGameDataSubsystem::LobbyAssetType = FPrimaryAssetType(TEXT("LobbyData"));
const FPrimaryAssetType UEDGameDataSubsystem::UIAssetType = FPrimaryAssetType(TEXT("UIData"));
const FPrimaryAssetType UEDGameDataSubsystem::ItemAssetType = FPrimaryAssetType(TEXT("InventoryItem"));
const FPrimaryAssetType UEDGameDataSubsystem::MonsterAssetType = FPrimaryAssetType(TEXT("MonsterData"));
const FPrimaryAssetType UEDGameDataSubsystem::PlayerDataAssetType = FPrimaryAssetType(TEXT("PlayerData"));
const FPrimaryAssetType UEDGameDataSubsystem::PlayerAnimDataAssetType = FPrimaryAssetType(TEXT("PlayerAnimData"));
const FPrimaryAssetType UEDGameDataSubsystem::WeaponDataAssetType = FPrimaryAssetType(TEXT("WeaponData"));

void UEDGameDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadPhase_Lobby();
}

void UEDGameDataSubsystem::Deinitialize()
{
	UnloadAllData();
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
	if (CurrentPhase != EDataLoadPhase::LobbyReady) return;
	LoadPhase_UI();
	UnloadPhaseData(LobbyAssetType, LobbyAssetType.GetName());
}

// ================================================================
// 언로드
// ================================================================

// 게임 종료 후 로비로
void UEDGameDataSubsystem::ReturnToLobby()
{
	// 게임씬 데이터 언로드
	UnloadPhaseData(UIAssetType,            UIAssetType.GetName());
	UnloadPhaseData(ItemAssetType,          ItemAssetType.GetName());
	UnloadPhaseData(MonsterAssetType,       MonsterAssetType.GetName());
	UnloadPhaseData(PlayerDataAssetType,    PlayerDataAssetType.GetName());
	UnloadPhaseData(PlayerAnimDataAssetType,PlayerAnimDataAssetType.GetName());
	UnloadPhaseData(WeaponDataAssetType,    WeaponDataAssetType.GetName());
	LoadPhase_Lobby();
}

// 게임 완전히 종료
void UEDGameDataSubsystem::UnloadAllData()
{
	UnloadPhaseData(LobbyAssetType,         LobbyAssetType.GetName());
	UnloadPhaseData(UIAssetType,            UIAssetType.GetName());
	UnloadPhaseData(ItemAssetType,          ItemAssetType.GetName());
	UnloadPhaseData(MonsterAssetType,       MonsterAssetType.GetName());
	UnloadPhaseData(PlayerDataAssetType,    PlayerDataAssetType.GetName());
	UnloadPhaseData(PlayerAnimDataAssetType,PlayerAnimDataAssetType.GetName());
	UnloadPhaseData(WeaponDataAssetType,    WeaponDataAssetType.GetName());
	SetPhase(EDataLoadPhase::NotStarted);
}

// 해당 에셋 언로드
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
		OnLobbyDataLoaded();
		return;
	}
	
	TSharedPtr<FStreamableHandle> Handle = AM.LoadPrimaryAssetsAsync(
		Ids,
		{LobbyAssetType.GetName()},
		FStreamableDelegate::CreateUObject(this, &UEDGameDataSubsystem::OnLobbyDataLoaded)
	);
	
	PhaseHandles.Add(LobbyAssetType.GetName(), Handle);
}

void UEDGameDataSubsystem::LoadPhase_UI()
{
	SetPhase(EDataLoadPhase::LoadingUI);
	UEDAssetManager& AM = UEDAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(UIAssetType, Ids);
	if (Ids.IsEmpty())
	{
		OnUIDataLoaded();
		return;
	}
	
	TSharedPtr<FStreamableHandle> Handle = AM.LoadPrimaryAssetsAsync(
		Ids,
		{UIAssetType.GetName()},
		FStreamableDelegate::CreateUObject(this, &UEDGameDataSubsystem::OnUIDataLoaded)
	);
	PhaseHandles.Add(UIAssetType.GetName(), Handle);
}

void UEDGameDataSubsystem::LoadPhase_Item()
{
	SetPhase(EDataLoadPhase::LoadingItem);
	UEDAssetManager& AM = UEDAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(ItemAssetType, Ids);
	if (Ids.IsEmpty())
	{
		OnItemDataLoaded();
		return;
	}
	
	TSharedPtr<FStreamableHandle> Handle = AM.LoadPrimaryAssetsAsync(
		Ids,
		{ItemAssetType.GetName()},
		FStreamableDelegate::CreateUObject(this, &UEDGameDataSubsystem::OnItemDataLoaded)
	);
	PhaseHandles.Add(ItemAssetType.GetName(), Handle);
}

void UEDGameDataSubsystem::LoadPhase_Monster()
{
	SetPhase(EDataLoadPhase::LoadingMonster);
	UEDAssetManager& AM = UEDAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	AM.GetPrimaryAssetIdList(MonsterAssetType, Ids);
	if (Ids.IsEmpty())
	{
		OnMonsterDataLoaded();
		return;
	}

	TSharedPtr<FStreamableHandle> Handle = AM.LoadPrimaryAssetsAsync(
		Ids,
		{MonsterAssetType.GetName()},
		FStreamableDelegate::CreateUObject(this, &UEDGameDataSubsystem::OnMonsterDataLoaded)
	);
	PhaseHandles.Add(MonsterAssetType.GetName(), Handle);
}

void UEDGameDataSubsystem::LoadPhase_Player()
{
	SetPhase(EDataLoadPhase::LoadingPlayer);
	UEDAssetManager& AM = UEDAssetManager::Get();

	TArray<FPrimaryAssetId> AllIds;
	TArray<FPrimaryAssetId> Temp;
	AM.GetPrimaryAssetIdList(PlayerDataAssetType, Temp);     AllIds.Append(Temp); Temp.Reset();
	AM.GetPrimaryAssetIdList(PlayerAnimDataAssetType, Temp); AllIds.Append(Temp); Temp.Reset();
	AM.GetPrimaryAssetIdList(WeaponDataAssetType, Temp);     AllIds.Append(Temp);

	if (AllIds.IsEmpty())
	{
		OnPlayerDataLoaded();
		return;
	}
	
	// 1단계: 번들 없이 DA 자체만 먼저 로드
	TSharedPtr<FStreamableHandle> Handle = AM.LoadPrimaryAssetsAsync(
		AllIds,
		{},  // 번들 없음
		FStreamableDelegate::CreateLambda([this, AllIds]()
		{
			// 2단계: DA 로드 완료 후 번들 상태 변경으로 TSoft 필드 로드
			UEDAssetManager& AM2 = UEDAssetManager::Get();
			TArray<FName> Bundles = {
				PlayerDataAssetType.GetName(),
				PlayerAnimDataAssetType.GetName(),
				WeaponDataAssetType.GetName()
			};

			AM2.ChangeBundleStateForPrimaryAssets(
				AllIds,
				Bundles,   // 추가할 번들
				{},        // 제거할 번들
				false,
				FStreamableDelegate::CreateUObject(this, &UEDGameDataSubsystem::OnPlayerDataLoaded)
			);
		})
	);
	
	PhaseHandles.Add(PlayerDataAssetType.GetName(), Handle);
	PhaseHandles.Add(PlayerAnimDataAssetType.GetName(), Handle);
	PhaseHandles.Add(WeaponDataAssetType.GetName(), Handle);
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
}

void UEDGameDataSubsystem::OnUIDataLoaded()
{
	CacheLoadedAssets(UIAssetType);
	LoadPhase_Item();
}

void UEDGameDataSubsystem::OnItemDataLoaded()
{
	CacheLoadedAssets(ItemAssetType);
	LoadPhase_Monster();
}

void UEDGameDataSubsystem::OnMonsterDataLoaded()
{
	CacheLoadedAssets(MonsterAssetType);
	LoadPhase_Player();
}

void UEDGameDataSubsystem::OnPlayerDataLoaded()
{
	CacheLoadedAssets(PlayerDataAssetType);
	CacheLoadedAssets(PlayerAnimDataAssetType);
	CacheLoadedAssets(WeaponDataAssetType);
	
	for (auto it:DataCache)
	{
		UE_LOG(LogTemp,Warning,TEXT("%s"),*it.Key.ToString());
		if (IsValid(it.Value))
		{
			UE_LOG(LogTemp,Warning,TEXT("IsValid"));
		}
	}
	
	
	
	SetPhase(EDataLoadPhase::Completed);
	UE_LOG(LogTemp, Log, TEXT("[EDGameDataSubsystem] 플레이어 데이터 로드 완료"));
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
		if (!Obj) continue;
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