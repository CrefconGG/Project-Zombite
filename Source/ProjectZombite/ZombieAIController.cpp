
#include "ZombieAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionSystem.h"
#include "Kismet/GameplayStatics.h"

AZombieAIController::AZombieAIController()
{
    SetPerceptionComponent(
        *CreateDefaultSubobject<UAIPerceptionComponent>(
            TEXT("AI Perception")
        )
    );

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(
        TEXT("SightConfig")
    );

    SightConfig->SightRadius = 1200.0f;
    SightConfig->LoseSightRadius = 1500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 60.0f;

    // Prototype uses Actor Tags, not Team IDs.
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    GetPerceptionComponent()->ConfigureSense(*SightConfig);

    GetPerceptionComponent()->SetDominantSense(
        UAISense_Sight::StaticClass()
    );
}

void AZombieAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!HasAuthority())
    {
        return;
    }

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }

    GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(
        this,
        &AZombieAIController::OnTargetSensed
    );
}

void AZombieAIController::OnTargetSensed(
    AActor* Actor,
    FAIStimulus Stimulus
)
{
    if (!Actor || !Actor->ActorHasTag(TEXT("Human")))
    {
        return;
    }

    UBlackboardComponent* BB = GetBlackboardComponent();

    if (!BB)
    {
        return;
    }

    const TSubclassOf<UAISense> Sense =
        UAIPerceptionSystem::GetSenseClassForStimulus(
            this, Stimulus
        );

    if (Sense != UAISense_Sight::StaticClass())
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        BB->SetValueAsObject(TargetActorKey, Actor);
    }
    else if (BB->GetValueAsObject(TargetActorKey) == Actor)
    {
        BB->ClearValue(TargetActorKey);
    }
}


bool AZombieAIController::TryBite(AActor* Target)
{
    if (!HasAuthority() || !IsValid(Target) ||
        !Target->ActorHasTag(TEXT("Human")))
    {
        return false;
    }

    APawn* Zombie = GetPawn();
    if (!Zombie)
    {
        return false;
    }

    const float Distance = FVector::Dist(
        Zombie->GetActorLocation(),
        Target->GetActorLocation()
    );

    if (Distance > BiteRange)
    {
        return false;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();

    if (CurrentTime - LastBiteTime < BiteCooldown)
    {
        return false;
    }

    LastBiteTime = CurrentTime;

    UGameplayStatics::ApplyDamage(
        Target,
        BiteDamage,
        this,
        Zombie,
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Warning,
        TEXT("Zombie bit %s"),
        *GetNameSafe(Target));

    return true;
}

