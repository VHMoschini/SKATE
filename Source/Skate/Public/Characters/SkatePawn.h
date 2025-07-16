#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"
#include "Curves/CurveVector.h"
#include "SkatePawn.generated.h"

UCLASS()
class SKATE_API ASkatePawn : public APawn
{
	GENERATED_BODY()

public:
	ASkatePawn();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	UBoxComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* SkateMesh;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootSceneComponent;

	UPROPERTY(EditAnywhere, Category = "Skate|Push")
	UAnimMontage* PushMontage;

	UPROPERTY(EditAnywhere, Category = "Skate|Push")
	float PushImpulseDelay = 0.4f;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	float AccelerationRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	float DecelerationRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	float JumpForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	float SnapTraceOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	float SnapHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	float MaxTurnRate;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UCurveVector* TurnLeanCurve;


	UPROPERTY(EditAnywhere, Category = "Animations")
	float MaxTurnTime = 1.f;

	UPROPERTY(EditAnywhere, Category = "Animations")
	FRotator MeshNeutralRotation = FRotator(0.f, 90.f, 0.f);

	UPROPERTY(EditAnywhere, Category = "Skate|Jump")
	UAnimMontage* JumpMontage;

	UPROPERTY(EditAnywhere, Category = "Skate|Jump")
	UCurveVector* JumpSkateCurve;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BrakeStrength = 1200.f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float JumpStrength = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float Gravity = 2000.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	float JumpElapsedTime = 0.f;
	bool bIsJumpingAnim = false;

	FTimerHandle PushTimerHandle;
	bool bIsPushing = false;

	bool bIsBraking = false;

	float TurnInputValue = 0.f;
	float TurnTime = 0.f;
	bool bIsTurning = false;

	float TimeSinceJump = 0.f;
	float GroundLockDuration = 0.2f;

	FVector Velocity;
	float CurrentSpeed;
	float VerticalVelocity = 0.f;

	bool bIsGrounded;

	void TurnRight(float Value);
	void Push();
	void StartBrake();
	void StopBrake();
	void Jump();
	void ApplyAnimations();

	void ApplyFriction(float DeltaTime);
	void CheckGround();

	void AlignToGround();
	void ApplyPushImpulse();
};
