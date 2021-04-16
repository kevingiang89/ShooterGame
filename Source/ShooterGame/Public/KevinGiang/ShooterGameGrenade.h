#pragma once

#include "GameFramework/Actor.h"

#include "ShooterGameGrenade.generated.h"

/**
 * Audio settings for the grenade's explosion SFX 
 */
USTRUCT()
struct FShooterGameGrenadeSoundSettings
{
    GENERATED_BODY()

public:
    /** Sound effect to play upon detonation */
    UPROPERTY(Category = "Grenade", EditAnywhere)
    class USoundCue* SoundCue;

    UPROPERTY(Category = "Grenade", EditAnywhere)
    float VolumeMultiplier;

    UPROPERTY(Category = "Grenade", EditAnywhere)
    float PitchMultiplier;

    UPROPERTY(Category = "Grenade", EditAnywhere)
    class USoundAttenuation* AttenuationSettings;

    UPROPERTY(Category = "Grenade", EditAnywhere)
    class USoundConcurrency* Concurrency;

    FShooterGameGrenadeSoundSettings()
    {
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        AttenuationSettings = nullptr;
        AttenuationSettings = nullptr;
    }
};

UCLASS()
class AShooterGameGrenade : public AActor
{
    GENERATED_BODY()

public:
    AShooterGameGrenade(const FObjectInitializer& ObjectInitializer);

#if 1 // Actor Interface
protected:
    virtual void BeginPlay() override;
#endif // Actor Interface

#if 1 // Shooter Game Grenade
private:
    UPROPERTY(Category = "Grenade", EditAnywhere, meta = (AllowPrivateAccess))
    class UStaticMeshComponent* GrenadeStaticMeshComponent;

    UPROPERTY(Category = "Grenade", EditAnywhere, meta = (AllowPrivateAccess))
    class USphereComponent* ExplosionRadiusSphereComponent;

#endif // Shooter Game Grenade

#if 1 // Gameplay
private:
    /** Damage curve for scaling damage based on distance */
    UPROPERTY(Category = "Grenade|Gameplay", EditAnywhere)
    UCurveFloat* DamageCurve;

    /** Seconds to wait before explosion occurs after the grenade has been spawned */
    UPROPERTY(Category = "Grenade|Gameplay", EditAnywhere)
    float DetonationDelay;

    /** Timer to manage detonation delay */
    FTimerHandle DetonationTimer;

private:
    /** 
     * Detonates the grenade and deals damage to all damageable 
     * actors within the damage curve radius 
     */
    UFUNCTION()
    void Detonate();
#endif // Gameplay

#if 1 // SFX
private:
    UPROPERTY(Category = "Grenade|SFX", EditAnywhere)
    FShooterGameGrenadeSoundSettings ExplosionSoundSettings;
#endif // SFX
};