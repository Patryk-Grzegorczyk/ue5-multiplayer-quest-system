#pragma once

#include "CoreMinimal.h"
#include "QuestTypes.generated.h"

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    None,
    FindLocation,
    FindSpecialItem,
    Labour
};

USTRUCT(BlueprintType)
struct FQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuestType QuestType = EQuestType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<AActor>> QuestActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<bool> QuestProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bCanBeCompleted = false;

    bool IsComplete() const
    {
        for (const bool Progress : QuestProgress)
        {
            if (!Progress)
            {
                return false;
            }
        }

        return QuestProgress.Num() > 0;
    }
};
