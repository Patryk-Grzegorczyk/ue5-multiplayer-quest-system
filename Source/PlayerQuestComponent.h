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

    UPROPERTY(BlueprintAssignable, Category="Quest")
    FOnQuestStateChanged OnQuestStateChanged;

    UFUNCTION(Server, Reliable)
    void AddQuest(FQuestData NewQuest);

    UFUNCTION(Server, Reliable)
    void FinishQuest(FName QuestName);

    bool UpdateQuestProgress(FName QuestName, int32 ProgressIndex);

    // These RPCs run on the owning player's client.
    UFUNCTION(Client, Reliable)
    void ClientSetQuestActorVisibility(AQuestActor* QuestActor, bool bVisible);

    UFUNCTION(Client, Reliable)
    void ClientDisableQuestActorInteraction(AQuestActor* QuestActor);

protected:
    virtual void BeginPlay() override;

    UFUNCTION(Client, Reliable)
    void StartQuests();

    UFUNCTION()
    void OnRep_ActiveQuests();

    UFUNCTION()
    void OnRep_FinishedQuests();

    void ShowQuestActors(const FQuestData& Quest);
    void HideAllQuestActors();
    FQuestData* FindQuest(FName QuestName);

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
