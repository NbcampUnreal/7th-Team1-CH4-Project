#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EDTestBlueprintLibrary.generated.h"

class AActor;
class UObject;
class UEDInventoryComponent;

UCLASS()
class ETERNALDREAMS_API UEDTestBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "ED|Test|Actor")
    static AActor* GetClosestActorOfClass(AActor* SourceActor, TSubclassOf<AActor> ActorClass, bool bIncludeSelf = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Actor")
    static AActor* GetFarthestActorOfClass(AActor* SourceActor, TSubclassOf<AActor> ActorClass, bool bIncludeSelf = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Inventory", meta = (WorldContext = "WorldContextObject"))
    static FString BuildAllInventoryDebugText(const UObject* WorldContextObject, bool bOnlyAuthority = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Inventory")
    static FString BuildInventoryDebugTextFromArray(const TArray<UEDInventoryComponent*>& InventoryComponents, bool bOnlyAuthority = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Inventory", meta = (WorldContext = "WorldContextObject"))
    static void GetAllWorldInventoryComponents(const UObject* WorldContextObject, TArray<UEDInventoryComponent*>& OutInventoryComponents, bool bOnlyAuthority = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Inventory", meta = (WorldContext = "WorldContextObject"))
    static void PrintAllInventoryDebugText(const UObject* WorldContextObject, float Duration = 0.0f, bool bOnlyAuthority = false);
};
