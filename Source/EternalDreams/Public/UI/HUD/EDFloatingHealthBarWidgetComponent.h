#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "EDFloatingHealthBarWidgetComponent.generated.h"

class UEDFloatingHealthBarWidget;

UCLASS(ClassGroup=(UI), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UEDFloatingHealthBarWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	virtual void InitWidget() override;

protected:
	virtual void BeginPlay() override;

private:
	void InitializeFromOwner();

	bool bSourceBound = false;
};
