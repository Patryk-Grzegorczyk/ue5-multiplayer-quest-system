#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestTypes.h"
#include "PlayerQuestComponent.generated.h"

class AQuestActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestStateChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QUESTSYSTEM_API UPlayerQuestComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerQuestComponent();

    UPROPERTY(ReplicatedUsing=OnRep_ActiveQuests, VisibleAnywhere, BlueprintReadOnly)
    TArray<FQuestData> ActiveQuests;

    UPROPERTY(ReplicatedUsing=OnRep_FinishedQuests, VisibleAnywhere, BlueprintReadOnly)
    TArray<FName> FinishedQuests;

    UPROPERTY(BlueprintAssignable)
    FOnQuestStateChanged OnQuestStateChanged;

    UFUNCTION(Server, Reliable)
    void AddQuest(const FQuestData& NewQuest);

    UFUNCTION(Server, Reliable)
    void CompleteQuest(const FName QuestName);

    bool UpdateQuestProgress(const FName QuestName, int32 ProgressIndex);

protected:
    UFUNCTION()
    void OnRep_ActiveQuests();

    UFUNCTION()
    void OnRep_FinishedQuests();

    UFUNCTION(Client, Reliable)
    void StartQuests();

    UFUNCTION(Client, Reliable)
    void SetQuestActorVisibility(AQuestActor* QuestActor, bool bVisible);

    void ShowQuestActors(const FQuestData& Quest);
    void HideAllQuestActors();
    FQuestData* FindQuest(const FName QuestName);

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
