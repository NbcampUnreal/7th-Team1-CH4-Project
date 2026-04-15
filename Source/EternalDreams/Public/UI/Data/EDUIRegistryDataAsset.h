#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/Types/EDUITypes.h"
#include "EDUIRegistryDataAsset.generated.h"

// UI가 어떤 종류인지 구분하기 위한 Enum
UENUM(BlueprintType)
enum class EEDUIWidgetType : uint8
{
	HUD UMETA(DisplayName = "HUD"),
	Panel UMETA(DisplayName = "Panel")
};

// UI Registry에 등록될 위젯 한 개의 정보
USTRUCT(BlueprintType)
struct FEDUIRegistryEntry
{
	GENERATED_BODY()

	// 위젯을 식별하기 위한 고유 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Registry")
	FName WidgetId = NAME_None;

	// 이 위젯이 HUD인지 Panel인지 구분
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Registry")
	EEDUIWidgetType WidgetType = EEDUIWidgetType::Panel;

	// 패널이 붙을 UI 레이어 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Registry")
	EEDUILayer Layer = EEDUILayer::Game;

	// 실제 생성에 사용할 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Registry", meta=(AssetBundles="UI"))
	TSoftClassPtr<UUserWidget> WidgetClass;
};

UCLASS()
class ETERNALDREAMS_API UEDUIRegistryDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 등록된 전체 UI 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Registry")
	TArray<FEDUIRegistryEntry> Entries;

	// WidgetId로 등록 정보를 찾는 함수, 없으면 nullptr를 반환
	const FEDUIRegistryEntry* FindEntryById(FName InWidgetId) const;

	// Registry 내부 데이터가 올바른지 검사
	bool ValidateEntries() const;
};
