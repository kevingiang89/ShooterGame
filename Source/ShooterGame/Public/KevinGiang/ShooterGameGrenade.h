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

    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // Actor Interface

#if 1 // Shooter Game Grenade
private:
    UPROPERTY(Category = "Grenade", EditAnywhere, meta = (AllowPrivateAccess))
    class UStaticMeshComponent* GrenadeStaticMeshComponent;

    /** 
     * Collision component for handling damage against other actors 
     * Radius is affected by DamageCurve where the max X-Value is the Radius
     */
    UPROPERTY()
    class USphereComponent* ExplosionRadiusSphereComponent;
#endif // Shooter Game Grenade

#if 1 // Gameplay
private:
    /**
     * Damage curve for scaling damage based on distance 
     * X-Value: Radius in Unreal Units
     * Y-Value: Damage to apply
     */
    UPROPERTY(Category = "Grenade|Gameplay", EditAnywhere)
    UCurveFloat* DamageCurve;

    /** Seconds to wait before explosion occurs after the grenade has been spawned */
    UPROPERTY(Category = "Grenade|Gameplay", EditAnywhere)
    float DetonationDelay;

    /** Timer to manage detonation delay */
    FTimerHandle DetonationTimer;

    UPROPERTY(Category = "Grenade|Gameplay", EditAnywhere, meta = (AllowPrivateAccess))
    TEnumAsByte<ECollisionChannel> BlastTraceChannel;

private:
    /** 
     * Detonates the grenade and deals damage to all damageable 
     * actors within the damage curve radius 
     */
    UFUNCTION()
    void Detonate();
#endif // Gameplay

#if 1 // VFX/SFX
private:
    UPROPERTY(Category = "Grenade|VFX", EditAnywhere)
    class UNiagaraSystem* ExplosionVFX;

    UPROPERTY(Category = "Grenade|SFX", EditAnywhere)
    FShooterGameGrenadeSoundSettings ExplosionSoundSettings;
#endif // SFX

#if 1 // Debuggger
private:
    /** Flag for enabling debug features for the grenade */
    UPROPERTY(Category = "Grenade|Debug", EditAnywhere)
    bool bEnableDebug;
#endif // Debugger
};