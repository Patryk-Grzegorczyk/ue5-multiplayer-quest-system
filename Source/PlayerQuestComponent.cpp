#include "PlayerQuestComponent.h"
#include "QuestActor.h"

#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

UPlayerQuestComponent::UPlayerQuestComponent()
{
    SetIsReplicatedByDefault(true);
}

void UPlayerQuestComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPlayerQuestComponent::AddQuest_Implementation(FQuestData NewQuest)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    if (NewQuest.QuestName.IsNone() || FindQuest(NewQuest.QuestName))
    {
        return;
    }

    ActiveQuests.Add(MoveTemp(NewQuest));
    StartQuests();
}

void UPlayerQuestComponent::FinishQuest_Implementation(FName QuestName)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    FQuestData* Quest = FindQuest(QuestName);
    if (!Quest || !Quest->IsComplete())
    {
        return;
    }

    FinishedQuests.AddUnique(QuestName);
    ActiveQuests.RemoveAll(
        [QuestName](const FQuestData& QuestEntry)
        {
            return QuestEntry.QuestName == QuestName;
        });

    StartQuests();
}

bool UPlayerQuestComponent::UpdateQuestProgress(
    FName QuestName,
    int32 ProgressIndex)
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
        FinishQuest(QuestName);
    }

    return true;
}

FQuestData* UPlayerQuestComponent::FindQuest(FName QuestName)
{
    return ActiveQuests.FindByPredicate(
        [QuestName](const FQuestData& Quest)
        {
            return Quest.QuestName == QuestName;
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
    if (!GetWorld())
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
    if (!GetWorld())
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
