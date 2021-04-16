#include "KevinGiang/ShooterGameGrenade.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"

AShooterGameGrenade::AShooterGameGrenade(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    GrenadeStaticMeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "GrenadeStaticMeshComponent");
    SetRootComponent(GrenadeStaticMeshComponent);

    ExplosionRadiusSphereComponent = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, "ExplosionRadiusSphereComponent");
    ExplosionRadiusSphereComponent->SetRelativeTransform(FTransform::Identity);
    ExplosionRadiusSphereComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}

void AShooterGameGrenade::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld())
    {
        if (DetonationDelay > 0.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(DetonationTimer, this, &AShooterGameGrenade::Detonate, DetonationDelay);
        }
        else
        {
            Detonate();
        }
    }
}

void AShooterGameGrenade::Detonate()
{
    // Damage curve is missing, no damage can be dealt!
    if (DamageCurve == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("AShooterGameGrenade::Detonate Missing damage curve! No damage can be dealt!"));
        return;
    }

    TSet<AActor*> OverlappedActors;
    ExplosionRadiusSphereComponent->GetOverlappingActors(OverlappedActors);


    for (AActor* OverlappedActor : OverlappedActors)
    {
        const float DistanceToActor = FVector::Distance(GetActorLocation(), OverlappedActor->GetActorLocation());
        const float DamageAmount = DamageCurve->GetFloatValue(DistanceToActor);

        FDamageEvent DamageEvent;
        OverlappedActor->TakeDamage(DamageAmount, DamageEvent, nullptr, nullptr);
    }

    if (ExplosionSoundSettings.SoundCue)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ExplosionSoundSettings.SoundCue,
            GetActorLocation(),
            ExplosionSoundSettings.VolumeMultiplier,
            ExplosionSoundSettings.PitchMultiplier,
            0.0f,
            ExplosionSoundSettings.AttenuationSettings,
            ExplosionSoundSettings.Concurrency
        );
    }
}
