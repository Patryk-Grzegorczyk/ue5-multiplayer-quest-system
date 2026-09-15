#include "QuestActor.h"
#include "PlayerQuestComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

AQuestActor::AQuestActor()
{
    bReplicates = true;

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    RootComponent = StaticMesh;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    CollisionBox->SetupAttachment(RootComponent);

    CollisionBox->OnComponentBeginOverlap.AddDynamic(
        this, &AQuestActor::OnOverlapBegin);
    CollisionBox->OnComponentEndOverlap.AddDynamic(
        this, &AQuestActor::OnOverlapEnd);

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

    QuestComponent->UpdateQuestProgress(QuestName, QuestProgressIndex);

    if (QuestType == EQuestType::FindSpecialItem)
    {
        ClientDisableInteraction();
        ClientSetVisibility(false);
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

    FTimerDelegate Delegate;
    Delegate.BindUFunction(
        this,
        FName("ProcessQuest"),
        PlayerController);

    FTimerHandle& TimerHandle = ActivePlayerTimers.FindOrAdd(PlayerController);

    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        Delegate,
        LabourWaitTime,
        false);
}

void AQuestActor::CancelLabour(APlayerController* PlayerController)
{
    if (!HasAuthority() || !PlayerController)
    {
        return;
    }

    if (FTimerHandle* TimerHandle = ActivePlayerTimers.Find(PlayerController))
    {
        GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
        ActivePlayerTimers.Remove(PlayerController);
    }
}

void AQuestActor::SetPlayerVisibility(bool bVisible)
{
    if (!HasAuthority())
    {
        return;
    }

    ClientSetVisibility(bVisible);
}

void AQuestActor::ClientSetVisibility_Implementation(bool bVisible)
{
    SetActorHiddenInGame(!bVisible);
    SetActorEnableCollision(bVisible);
}

void AQuestActor::ClientDisableInteraction_Implementation()
{
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AQuestActor::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority() || !OtherActor)
    {
        return;
    }

    if (QuestType != EQuestType::FindLocation)
    {
        return;
    }

    if (APawn* Pawn = Cast<APawn>(OtherActor))
    {
        if (APlayerController* PC = Cast<APlayerController>(
            Pawn->GetController()))
        {
            ProcessQuest(PC);
        }
    }
}

void AQuestActor::OnOverlapEnd(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex)
{
}
