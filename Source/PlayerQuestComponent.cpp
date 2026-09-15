#include "PlayerQuestComponent.h"
#include "QuestActor.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

UPlayerQuestComponent::UPlayerQuestComponent()
{
    SetIsReplicatedByDefault(true);
}

void UPlayerQuestComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPlayerQuestComponent::AddQuest_Implementation(const FQuestData& NewQuest)
{
    if (!NewQuest.QuestName.IsNone())
    {
        ActiveQuests.Add(NewQuest);
        StartQuests();
    }
}

void UPlayerQuestComponent::CompleteQuest_Implementation(const FName QuestName)
{
    FQuestData* Quest = FindQuest(QuestName);
    if (!Quest || !Quest->IsComplete())
    {
        return;
    }

    FinishedQuests.AddUnique(QuestName);
    ActiveQuests.RemoveAll(
        [QuestName](const FQuestData& Entry)
        {
            return Entry.QuestName == QuestName;
        });

    StartQuests();
}

bool UPlayerQuestComponent::UpdateQuestProgress(const FName QuestName, int32 ProgressIndex)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return false;
    }

    FQuestData* Quest = FindQuest(QuestName);
    if (!Quest || !Quest->QuestProgress.IsValidIndex(ProgressIndex))
    {
        return false;
    }

    if (Quest->QuestProgress[ProgressIndex])
    {
        return false;
    }

    Quest->QuestProgress[ProgressIndex] = true;
    Quest->bCanBeCompleted = Quest->IsComplete();

    OnQuestStateChanged.Broadcast();

    if (Quest->bCanBeCompleted)
    {
        CompleteQuest(QuestName);
    }

    return true;
}

FQuestData* UPlayerQuestComponent::FindQuest(const FName QuestName)
{
    return ActiveQuests.FindByPredicate(
        [QuestName](const FQuestData& Entry)
        {
            return Entry.QuestName == QuestName;
        });
}

void UPlayerQuestComponent::StartQuests_Implementation()
{
    HideAllQuestActors();

    for (const FQuestData& Quest : ActiveQuests)
    {
        ShowQuestActors(Quest);
    }
}

void UPlayerQuestComponent::HideAllQuestActors()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    for (TActorIterator<AQuestActor> It(GetWorld()); It; ++It)
    {
        SetQuestActorVisibility(*It, false);
    }
}

void UPlayerQuestComponent::ShowQuestActors(const FQuestData& Quest)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    for (const TSubclassOf<AActor>& ActorClass : Quest.QuestActors)
    {
        if (!ActorClass)
        {
            continue;
        }

        for (TActorIterator<AQuestActor> It(GetWorld(), ActorClass); It; ++It)
        {
            SetQuestActorVisibility(*It, true);
        }
    }
}

void UPlayerQuestComponent::SetQuestActorVisibility_Implementation(
    AQuestActor* QuestActor,
    bool bVisible)
{
    if (!QuestActor)
    {
        return;
    }

    QuestActor->SetPlayerVisibility(bVisible);
}

void UPlayerQuestComponent::OnRep_ActiveQuests()
{
    OnQuestStateChanged.Broadcast();
}

void UPlayerQuestComponent::OnRep_FinishedQuests()
{
    OnQuestStateChanged.Broadcast();
}

void UPlayerQuestComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPlayerQuestComponent, ActiveQuests);
    DOREPLIFETIME(UPlayerQuestComponent, FinishedQuests);
}
