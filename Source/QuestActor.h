#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "QuestTypes.h"
#include "QuestActor.generated.h"

class UPlayerQuestComponent;

UCLASS()
class QUESTSYSTEM_API AQuestActor : public AActor
{
    GENERATED_BODY()

public:
    AQuestActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* StaticMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UBoxComponent* CollisionBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
    FName QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
    int32 QuestProgressIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
    EQuestType QuestType = EQuestType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Labour")
    float LabourWaitTime = 0.0f;

    UFUNCTION()
    void ProcessQuest(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable)
    void BeginLabour(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable)
    void CancelLabour(APlayerController* PlayerController);

    void SetPlayerVisibility(bool bVisible);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComponent,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComponent,
        int32 OtherBodyIndex);

    UFUNCTION(Client, Reliable)
    void ClientSetVisibility(bool bVisible);

    UFUNCTION(Client, Reliable)
    void ClientDisableInteraction();

    TMap<APlayerController*, FTimerHandle> ActivePlayerTimers;
};
