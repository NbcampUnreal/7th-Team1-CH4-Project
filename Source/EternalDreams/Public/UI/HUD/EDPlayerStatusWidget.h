#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "EDPlayerStatusWidget.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class ETERNALDREAMS_API UEDPlayerStatusWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 플레이어 이름 표시용 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> PlayerNameText;
	
	// 레벨 표시용 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> LevelText;
	
	// HP 표시 바
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UProgressBar> HPBar;
	
	// HP 수치 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> HPValueText;
	
	// 마나 표시 바
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UProgressBar> ManaBar;
	
	// 마나 수치 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> ManaValueText;
	
	// 상태 표시용 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> StatusText;

private:
	// 더미 값 표시용
	void ApplyTestData() const;
};
