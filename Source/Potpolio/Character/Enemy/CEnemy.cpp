#include "CEnemy.h"
#include "Global.h"
#include "Components/CapsuleComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CMontageComponent.h"
#include "Component/CAttributeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "AIController.h"
#include "Blueprint/UserWidget.h"

ACEnemy::ACEnemy()
{
	GetMesh()->SetRelativeLocation(FVector(0, 0, -88));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
	USkeletalMesh* MeshComp;
	ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT("/Game/Enemy/Mesh/Swat"));
	if (MeshAsset.Succeeded())
	{
		MeshComp = MeshAsset.Object;
		GetMesh()->SetSkeletalMesh(MeshComp);
	}

	//ActionComponet
	CHelpers::CreateActorComponent(this, &ActionComp, TEXT("ActionComp"));
	//MontageComponet
	CHelpers::CreateActorComponent(this, &MontageComp, TEXT("MontageComp"));
	//StateComponent
	CHelpers::CreateActorComponent(this, &StateComp, TEXT("StateComp"));
	//AttributeComponet
	CHelpers::CreateActorComponent(this, &AttributeComp, TEXT("Attribute"));
	//Widget
	CHelpers::CreateSceneComponent(this, &WidgetComp, TEXT("WidgetComp"),GetMesh());


	
	if (TargetWidgetClass)
	{
		WidgetComp->SetWidgetClass(TargetWidgetClass);
		
		//WidgetComp->SetVisibility(false);
	}

}

void ACEnemy::BeginPlay()
{
	Super::BeginPlay();

	StateComp->OnStateTypeChanged.AddDynamic(this, &ACEnemy::OnStateTypeChanged);

	AIC = GetController<AAIController>();
}

float ACEnemy::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	DamageValue = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	DamageInstigator = EventInstigator;



	//ActionComp->Abort();
	AttributeComp->OnDecreseHealth(Damage);

	if (AttributeComp->GetCurrentHealth() <= 0)
	{
		StateComp->SetDeadMode();
		return 0.0f;
	}

	StateComp->SetHittedMode();

	return DamageValue;
}

void ACEnemy::Hitted()
{
	MontageComp->PlayHitted();
}

void ACEnemy::Dead()
{
	MontageComp->PlayDead();
	GetMesh()->SetCollisionProfileName("Ragdoll");
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AIC->UnPossess();
}

void ACEnemy::TagetWidgetOn()
{
	CheckNull(WidgetComp);
	CLog::Print("Widget On");
	WidgetComp->SetVisibility(true);
}

void ACEnemy::TagetWidgetOff()
{
	CheckNull(WidgetComp);
	CLog::Print("Widget Off");
	WidgetComp->SetVisibility(false);
}

void ACEnemy::OnStateTypeChanged(EStateType PreType, EStateType NewType)
{
	switch (NewType)
	{
	
	case EStateType::Hitted:
	{
		Hitted();
		break;
	}
	case EStateType::Dead:
	{
		Dead();
		break;
	}
	default:
		break;
	}
}
