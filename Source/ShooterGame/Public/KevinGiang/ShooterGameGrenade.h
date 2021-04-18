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

#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif WITH_EDITOR
#endif // Actor Interface

#if 1 // Shooter Game Grenade
private:
    /** Mesh component for rendering the grenade */
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
public:
    /**
     * @return Return the amount of time remaining before the grenade explodes 
     */
    UFUNCTION(Category = "Grenade|Gameplay", BlueprintPure)
    float GetDetonationTimerRemaining() const;

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

    /** 
     * The trace channel that the grenade will trace against when detonating 
     * If this channel is blocked by an actor, then that actor can be used as cover
     */
    UPROPERTY(Category = "Grenade|Gameplay", EditAnywhere, meta = (AllowPrivateAccess))
    TEnumAsByte<ECollisionChannel> BlastTraceChannel;

private:
    /** 
     * Detonates the grenade and deals damage to all damageable 
     * actors within the damage curve radius 
     */
    UFUNCTION()
    void Detonate();

    void UpdateExplosionRadiusSphereComponent();
#endif // Gameplay

#if 1 // VFX/SFX
private:
    /** VFX to use when the grenade explodes */
    UPROPERTY(Category = "Grenade|VFX", EditAnywhere)
    class UNiagaraSystem* ExplosionVFX;

    /** Scale of the VFX that is spawned in world space */
    UPROPERTY(Category = "Grenade|VFX", EditAnywhere)
    FVector ExposionVFXScale;

    /** SFX and its settings to use when the grenade explodes */
    UPROPERTY(Category = "Grenade|SFX", EditAnywhere)
    FShooterGameGrenadeSoundSettings ExplosionSoundSettings;
#endif // SFX

#if 1 // Debuggger
public:
    UFUNCTION(Category = "Grenade|Gameplay", BlueprintPure)
    bool GetDebugEnabled() const;

private:
    /** Flag for enabling debug features for the grenade */
    UPROPERTY(Category = "Grenade|Debug", EditAnywhere)
    bool bEnableDebug;
#endif // Debugger
};