#include "CDoAction_Pistol.h"
#include "Global.h"
#include "GameFramework/Character.h"
#include "Component/CActionComponent.h"
#include "Component/CMontageComponent.h"
#include "Component/CStateComponent.h"
#include "Action/CAction.h"
#include "Action/CAttachment.h"
#include "Action/CBullet.h"
#include "Camera/CameraShake.h"
#include "DrawDebugHelpers.h"
#include "Aim.h"



ACDoAction_Pistol::ACDoAction_Pistol()
{
	Aim = CreateDefaultSubobject<UAim>("Aim");
	CHelpers::GetAsset(&ReloadSound, "/Game/Player/Animations/Sound/Pistol/ReloadPistoll_Cue");
	CHelpers::GetAsset(&AimSound, "/Game/Player/Animations/Sound/Pistol/AimPistol_Cue");
}

void ACDoAction_Pistol::BeginPlay()
{
	Super::BeginPlay();
	Aim->BeginPlay(OwnerCharacter);
	
	MontageComp = CHelpers::GetComponent<UCMontageComponent>(OwnerCharacter);

	

	ReloadBullet = Data[0].MaxBullet;
	MaxBulletCount = ReloadBullet;
	CurrentBulletCount = MaxBulletCount;
	
}

void ACDoAction_Pistol::DoAction()
{
	CheckNull(Data[0].Bullet);
	CheckFalse(bAiming);
	if (CurrentBulletCount <= 0)
	{
		return;
	}
	CurrentBulletCount--;

	CheckNull(ActionComp);
	ActionData = ActionComp->GetCurrentAction();
	CheckNull(ActionData);
	Attachment = ActionData->GetAttachment();
	CheckNull(Attachment);

	

	//Spawn Bullet
	FTransform Transform;
	
	FVector MuzzleLocation = Attachment->GetMesh()->GetSocketLocation("MuzzleFlash");
	
	FVector CamLoc;
	FRotator CamRot;

	PC->GetPlayerViewPoint(CamLoc, CamRot);

	//Traace
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	QueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape Shape;
	Shape.SetSphere(20.f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParam.Instigator = OwnerCharacter->GetInstigator();

	FHitResult TraceHit;
	FVector TraceStart = CamLoc;
	FVector TraceEnd = TraceStart + (CamRot.Vector() * 100000.f);

	if (GetWorld()->SweepSingleByObjectType(TraceHit, TraceStart, TraceEnd,FQuat::Identity ,QueryParams, Shape,Params))
	{
		 TraceEnd = TraceHit.ImpactPoint;
	}
	DrawDebugLine(GetWorld(),TraceStart,TraceEnd,FColor::Red,false,5.f);
	

	FRotator Rotation =  FRotationMatrix::MakeFromX( TraceEnd - MuzzleLocation).Rotator();
	FTransform SpawnTransform(Rotation, MuzzleLocation);

	ACbullet* Bullet = GetWorld()->SpawnActor<ACbullet>(Data[0].Bullet, SpawnTransform, SpawnParam);
	
	CheckNull(Bullet);
	//Bind BulletDelegate
	Bullet->OnBulletBeginOverlap.AddDynamic(this, &ACDoAction_Pistol::OnBulletBeginOverlap);

	//Swpan Particle
	UParticleSystem* Particle = Attachment->GetParticle();
	UGameplayStatics::SpawnEmitterAttached(Particle, Attachment->GetMesh(), "MuzzleFlash");

	//Play Sound
	USoundBase* PistolSound = Attachment->GetSound();
	
	UGameplayStatics::PlaySound2D(GetWorld(), PistolSound);


	//Play CameraShake
	CheckNull(Data[0].CameraShake);
	PC->PlayerCameraManager->StartMatineeCameraShake(Data[0].CameraShake);

	if (CurrentBulletCount <= 0)
	{
		CheckNull(MontageComp);
		StateComp->SetReloadMode();
		MontageComp->PlayReload();
	}
}

void ACDoAction_Pistol::SubDoAction(bool InbAiming)
{
	bAiming = InbAiming;
	Aim->SetVisiblity(InbAiming);
	if (bAiming)
	{
	UGameplayStatics::PlaySound2D(GetWorld(), AimSound);
	}
}

void ACDoAction_Pistol::SetAimBeginPlay()
{
	Aim->SetHUD(OwnerCharacter);
}

void ACDoAction_Pistol::OnBulletBeginOverlap(FHitResult InHitResult)
{
	FHitResult HitResult = InHitResult;
	float DamageAmount = Data[0].Power;
	if (Data[0].Particle)
	{
		FTransform EffectLocation = Data[0].EffectTransforms;
		EffectLocation.AddToTranslation(HitResult.ImpactPoint);
		EffectLocation.SetRotation(FQuat(HitResult.ImpactNormal.Rotation()));

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),Data[0].Particle, EffectLocation);
	}

	FDamageEvent DamageEvent;
	if (HitResult.BoneName == "Head")
	{
		DamageAmount *= 5.f;
	}
	HitResult.GetActor()->TakeDamage(DamageAmount,DamageEvent,PC,this);
}

void ACDoAction_Pistol::OnReload()
{
	
	if (MaxBulletCount <= 0)
	{
		return;
	}
	
	if ( (ReloadBullet - CurrentBulletCount) >= MaxBulletCount)
	{
		CurrentBulletCount += MaxBulletCount;
		MaxBulletCount = 0;
	}
	else
	{
	MaxBulletCount -= (ReloadBullet - CurrentBulletCount);
	CurrentBulletCount += (ReloadBullet - CurrentBulletCount);
	}
	ReloadBullet = Data[0].MaxBullet;
	UGameplayStatics::PlaySound2D(GetWorld(),ReloadSound);
}


