// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1ScentSphere.h"
#include "Components/SphereComponent.h"
#include "Characters/Killer/D1KillerBase.h"
#include "EngineUtils.h"

// Sets default values
AD1ScentSphere::AD1ScentSphere()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(60.f);
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Script/Engine.StaticMesh'/Game/Art/Map/Interactable/ScentSphere/Scent_Sphere.Scent_Sphere'")));
	Mesh->SetWorldScale3D(FVector(0.5f));
	Mesh->SetVisibility(true);
	Mesh->SetHiddenInGame(false);
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Collision->OnComponentBeginOverlap.AddDynamic(this, &AD1ScentSphere::OnOverlapBegin);
}

// Called when the game starts or when spawned
void AD1ScentSphere::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		Mesh->SetVisibility(false);
		Mesh->SetHiddenInGame(true);
	}

	for (TActorIterator<AD1KillerBase> It(GetWorld()); It; ++It)
	{
		CachedKiller = *It;
		break;
	}

	GetWorld()->GetTimerManager().SetTimer(LifetimeHandle, this, &AD1ScentSphere::DestroySelf, 8.f, false);
}

// Called every frame
void AD1ScentSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CachedKiller.IsValid())
	{
		if (CachedKiller->GetCurrentTransformState() != EDraculaTransformationState::Wolf)
		{
			Destroy();
		}
	}
}

void AD1ScentSphere::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (AD1KillerBase* Killer = Cast<AD1KillerBase>(OtherActor))
	{
		if (Killer->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
		{
			Killer->ApplySmellBuff();
			Destroy();
		}
	}
}

void AD1ScentSphere::DestroySelf()
{
	Destroy();
}
