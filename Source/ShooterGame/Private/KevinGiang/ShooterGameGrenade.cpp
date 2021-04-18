#include "KevinGiang/ShooterGameGrenade.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"

#include "NiagaraFunctionLibrary.h"

AShooterGameGrenade::AShooterGameGrenade(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    GrenadeStaticMeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "GrenadeStaticMeshComponent");
    SetRootComponent(GrenadeStaticMeshComponent);

    ExplosionRadiusSphereComponent = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, "ExplosionRadiusSphereComponent");
    ExplosionRadiusSphereComponent->SetRelativeTransform(FTransform::Identity);
    ExplosionRadiusSphereComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

    // Only care about pawns within the explosion radius
    ExplosionRadiusSphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    ExplosionRadiusSphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    BlastTraceChannel = ECC_GameTraceChannel4; // Should be set to Blast visibility channel

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

#if WITH_EDITOR
void AShooterGameGrenade::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (ExplosionRadiusSphereComponent)
    {
        // Show explosion radius in-game if debug mode is on
        ExplosionRadiusSphereComponent->SetHiddenInGame(!bEnableDebug);

        if (DamageCurve)
        {
            float MinRadius, MaxRadius;
            DamageCurve->GetTimeRange(MinRadius, MaxRadius);
            ExplosionRadiusSphereComponent->SetSphereRadius(MaxRadius);
        }
    }
}
#endif WITH_EDITOR

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

    // Don't trace against self
    FCollisionQueryParams CollisionQueryParams;
    CollisionQueryParams.AddIgnoredActor(this);

    for (AActor* OverlappedActor : OverlappedActors)
    {
        // Trace against the pawn to check if it was behind cover
        TArray<FHitResult> HitResults;
        GetWorld()->LineTraceMultiByChannel(
            HitResults,
            GetActorLocation(),
            OverlappedActor->GetActorLocation(),
            BlastTraceChannel,
            CollisionQueryParams
        );

        // If the first thing we hit is the overlapped actor, then damage is valid
        bool bDamageBlocked = true; // Assume blocked initially
        if ((HitResults.Num() > 0) && (HitResults[0].Actor == OverlappedActor))
        {
            bDamageBlocked = false;
        }

        if (bDamageBlocked == false)
        {
            const float DistanceToActor = FVector::Distance(GetActorLocation(), OverlappedActor->GetActorLocation());
            const float DamageAmount = DamageCurve->GetFloatValue(DistanceToActor);

            FDamageEvent DamageEvent;
            DamageEvent.DamageTypeClass = UDamageType::StaticClass();

            // Finally deal damage to the target
            OverlappedActor->TakeDamage(DamageAmount, DamageEvent, nullptr, nullptr);
        }
    }

    // Play one-shot VFX/SFX in level that is not bound to this actor so it can finish playing
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

        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this,
            ExplosionVFX,
            GetActorLocation()
        );
    }

    Destroy();
}
