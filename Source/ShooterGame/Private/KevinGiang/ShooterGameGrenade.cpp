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

    BlastTraceChannel = COLLISION_BLAST;
    ExposionVFXScale = FVector::OneVector;
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
    
    UpdateExplosionRadiusSphereComponent();
}

float AShooterGameGrenade::GetDetonationTimerRemaining() const
{
    float TimeRemaining = 0.0f;

    if (GetWorld())
    {
        TimeRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(DetonationTimer);
    }

    return TimeRemaining;
}

#if WITH_EDITOR
void AShooterGameGrenade::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Update sphere radius for debugging
    UpdateExplosionRadiusSphereComponent();
}
#endif WITH_EDITOR

void AShooterGameGrenade::UpdateExplosionRadiusSphereComponent()
{
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

        bool bDamageBlocked = true; // Assume blocked initially

        // Damage should not be blocked until we hit a non-Pawn actor AND
        // if we never reached the overlapped actor. This will account for
        // tracing past other pawns that we aren't interested in
        for (FHitResult HitResult : HitResults)
        {
            if (HitResult.Actor != nullptr)
            {
                // If we reached the overlapped actor, then we can damage it
                if (HitResult.Actor == OverlappedActor)
                {
                    bDamageBlocked = false;
                }
                // If the hitactor is not a pawn, then stop tracing
                else if (!HitResult.Actor->IsA<APawn>())
                {
                    break;
                }
            }
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
            GetActorLocation(),
            FRotator::ZeroRotator,
            ExposionVFXScale
        );
    }

    Destroy();
}

bool AShooterGameGrenade::GetDebugEnabled() const
{
    return bEnableDebug;
}