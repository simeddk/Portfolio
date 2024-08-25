#include "AI/CAIController.h"
#include "Global.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Component/CBehaviorComponent.h"
#include "Character/Enemy/CEnemy_AI.h"
#include "Character/Player/CPlayer.h"
#include "CAIController.h"
#include "DrawDebugHelpers.h"


ACAIController::ACAIController()
{

	PrimaryActorTick.bCanEverTick = true;

	BehaviorRange = 1000.f;
	bDrawRange = true;
	Segment = 32;

	CHelpers::CreateActorComponent<UBlackboardComponent>(this, &Blackboard, "BlackboardComp");
	CHelpers::CreateActorComponent<UCBehaviorComponent>(this, &BehaviorComp, "BehaviorComp");
	CHelpers::CreateActorComponent(this, &PerceptionComp, "PerceptionComp");
	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight");

	Sight->SightRadius = 2000.f;
	Sight->LoseSightRadius = 2500.f;
	Sight->PeripheralVisionAngleDegrees = 90.f;

	Sight->DetectionByAffiliation.bDetectEnemies = true;
	Sight->DetectionByAffiliation.bDetectFriendlies = false;
	Sight->DetectionByAffiliation.bDetectNeutrals = false;

	PerceptionComp->ConfigureSense(*Sight);
	PerceptionComp->SetDominantSense(Sight->GetSenseImplementation());
}

void ACAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwnerEnemy = Cast<ACEnemy_AI>(InPawn);

	SetGenericTeamId(OwnerEnemy->GetTeamID());
	ensure( OwnerEnemy->GetBehaviorTree());
	
	UseBlackboard(OwnerEnemy->GetBehaviorTree()->BlackboardAsset, Blackboard);
	BehaviorComp->SetBlackBoard(Blackboard);
	RunBehaviorTree(OwnerEnemy->GetBehaviorTree());
	
	PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ACAIController::OnPerceptionUpdated);
}

void ACAIController::OnUnPossess()
{
	PerceptionComp->OnPerceptionUpdated.Clear();
}

void ACAIController::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);

	FVector Center = OwnerEnemy->GetActorLocation();
	CheckFalse(bDrawRange);
	DrawDebugSphere(GetWorld(), Center, Sight->SightRadius, Segment, FColor::Green);
	DrawDebugSphere(GetWorld(), Center, BehaviorRange, Segment, FColor::Green);

}


void ACAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	TArray<AActor*> PerceivedActors;
	PerceptionComp->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

	ACPlayer* Player = nullptr;
	for (const auto& Actor : PerceivedActors)
	{
		Player = Cast<ACPlayer>(Actor);

		if (Player)
		{
			break;
		}
	}

	Blackboard->SetValueAsObject("PlayerKey", Player);
}

float ACAIController::GetSightRadius()
{
	return Sight->SightRadius;
}