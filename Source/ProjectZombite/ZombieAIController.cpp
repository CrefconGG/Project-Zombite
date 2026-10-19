
#include "ZombieAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AZombieAIController::AZombieAIController()
{
    SetPerceptionComponent(
        *CreateDefaultSubobject<UAIPerceptionComponent>(
            TEXT("AI Perception")
        )
    );

    // Sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(
        TEXT("SightConfig")
    );

    SightConfig->SightRadius = 1200.0f;
    SightConfig->LoseSightRadius = 1500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 60.0f;

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    GetPerceptionComponent()->ConfigureSense(*SightConfig);

    // Hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(
        TEXT("HearingConfig")
    );

    HearingConfig->HearingRange = 2000.0f;
    HearingConfig->SetMaxAge(5.0f);

    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

    GetPerceptionComponent()->ConfigureSense(*HearingConfig);
    
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

    if (UAIPerceptionComponent* Perception =
        GetPerceptionComponent())
    {
        Perception->OnTargetPerceptionUpdated.AddDynamic(
            this,
            &AZombieAIController::OnTargetSensed
        );
    }
}


void AZombieAIController::OnTargetSensed(
    AActor* Actor,
    FAIStimulus Stimulus
)
{
    if (!IsValid(Actor) || !HasAuthority())
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
            this,
            Stimulus
        );
    
    auto SetZombieSpeed = [this](float NewSpeed)
    {
        if (ACharacter* Zombie = Cast<ACharacter>(GetPawn()))
        {
            Zombie->GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
        }
    };

    // Sight
    if (Sense == UAISense_Sight::StaticClass())
    {
        if (!Actor->ActorHasTag(TEXT("Human")))
        {
            return;
        }

        if (Stimulus.WasSuccessfullySensed())
        {
            BB->SetValueAsObject(TargetActorKey, Actor);
            
            SetZombieSpeed(600.f);

            UE_LOG(LogTemp, Warning,
                TEXT("Zombie spotted Human: %s"),
                *GetNameSafe(Actor));
        }
        else if (BB->GetValueAsObject(TargetActorKey) == Actor)
        {
            BB->ClearValue(TargetActorKey);
            
            SetZombieSpeed(200.f);

            UE_LOG(LogTemp, Warning,
                TEXT("Zombie lost sight of Human: %s"),
                *GetNameSafe(Actor));
        }

        return;
    }

    // Hearing
    if (Sense == UAISense_Hearing::StaticClass())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            BB->SetValueAsVector(
                InvestigateLocationKey,
                Stimulus.StimulusLocation
            );

            UE_LOG(LogTemp, Warning,
                TEXT("Zombie heard noise at %s"),
                *Stimulus.StimulusLocation.ToString());
        }

        return;
    }
}

// Bite
bool AZombieAIController::TryBite(AActor* Target)
{
    if (!HasAuthority() ||
        !IsValid(Target) ||
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
