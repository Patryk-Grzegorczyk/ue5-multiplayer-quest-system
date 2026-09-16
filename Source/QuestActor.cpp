#include "QuestActor.h"
#include "PlayerQuestComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

AQuestActor::AQuestActor()
{
    bReplicates = true;

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    RootComponent = StaticMesh;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    CollisionBox->SetupAttachment(RootComponent);

    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AQuestActor::OnOverlapBegin);
    CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AQuestActor::OnOverlapEnd);

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}

void AQuestActor::BeginPlay()
{
    Super::BeginPlay();
}

void AQuestActor::ProcessQuest(APlayerController* PlayerController)
{
    if (!HasAuthority() || !PlayerController)
    {
        return;
    }

    APawn* Pawn = PlayerController->GetPawn();
    if (!Pawn)
    {
        return;
    }

    UPlayerQuestComponent* QuestComponent =
        Pawn->FindComponentByClass<UPlayerQuestComponent>();

    if (!QuestComponent)
    {
        return;
    }

    if (!QuestComponent->UpdateQuestProgress(QuestName, QuestProgressIndex))
    {
        return;
    }

    if (QuestType == EQuestType::FindSpecialItem)
    {
        QuestComponent->ClientSetQuestActorVisibility(this, false);
        QuestComponent->ClientDisableQuestActorInteraction(this);
    }

    ActivePlayerTimers.Remove(PlayerController);
}

void AQuestActor::BeginLabour(APlayerController* PlayerController)
{
    if (!HasAuthority() || !PlayerController)
    {
        return;
    }

    if (ActivePlayerTimers.Contains(PlayerController))
    {
        return;
    }

    if (LabourWaitTime <= 0.0f)
    {
        ProcessQuest(PlayerController);
        return;
    }

    FTimerDelegate InteractionDelegate;
    InteractionDelegate.BindUFunction(this, FName("ProcessQuest"), PlayerController);

    FTimerHandle& TimerHandle = ActivePlayerTimers.FindOrAdd(PlayerController);

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            InteractionDelegate,
            LabourWaitTime,
            false);
    }
}

void AQuestActor::CancelLabour(APlayerController* PlayerController)
{
    if (!HasAuthority() || !PlayerController)
    {
        return;
    }

    if (FTimerHandle* TimerHandle = ActivePlayerTimers.Find(PlayerController))
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
        }

        ActivePlayerTimers.Remove(PlayerController);
    }
}

void AQuestActor::SetPlayerVisibilityLocal(bool bVisible)
{
    SetActorHiddenInGame(!bVisible);
    SetActorEnableCollision(bVisible);
}

void AQuestActor::DisableInteractionLocal()
{
    if (CollisionBox)
    {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AQuestActor::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority() || !OtherActor || QuestType != EQuestType::FindLocation)
    {
        return;
    }

    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn)
    {
        return;
    }

    if (APlayerController* PlayerController = Pawn->GetController())
    {
        ProcessQuest(PlayerController);
    }
}

void AQuestActor::OnOverlapEnd(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex)
{
}
