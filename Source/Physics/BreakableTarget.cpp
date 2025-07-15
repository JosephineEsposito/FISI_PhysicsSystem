#include "BreakableTarget.h"
#include <GeometryCollection/GeometryCollectionComponent.h>

// Sets default values
ABreakableTarget::ABreakableTarget()
{
 	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	GeometryCollection->SetupAttachment(StaticMesh);
	GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableTarget::GeometryCollectionBroken);
	GeometryCollection->SetNotifyBreaks(true);
}

void ABreakableTarget::GeometryCollectionBroken(const FChaosBreakEvent& BreakEvent)
{
	if (!m_IsBroken)
	{
		OnTargetBroken.Broadcast(this);
		m_IsBroken = true;
	}
}

