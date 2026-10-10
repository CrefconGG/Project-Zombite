
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "ZombieAIController.generated.h"

class UBehaviorTree;
class UAISenseConfig_Sight;

UCLASS()
class PROJECTZOMBITE_API AZombieAIController : public AAIController
{
	GENERATED_BODY()

public:
	AZombieAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetActorKey = TEXT("TargetActor");

	UFUNCTION()
	void OnTargetSensed(AActor* Actor, FAIStimulus Stimulus);
	
	UPROPERTY(EditDefaultsOnly, Category = "Zombie Attack")
	float BiteRange = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Attack")
	float BiteDamage = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Attack")
	float BiteCooldown = 1.5f;

	float LastBiteTime = -1000.0f;

	UFUNCTION(BlueprintCallable, Category = "Zombie Attack")
	bool TryBite(AActor* Target);

};
