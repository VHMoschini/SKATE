#include "Characters/SkatePawn.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Math/RotationMatrix.h"

ASkatePawn::ASkatePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootSceneComponent;

	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetupAttachment(RootComponent);

	SkateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkateMesh"));
	SkateMesh->SetupAttachment(Collision);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	VerticalVelocity = 0.f;
	JumpStrength = 600.f;
	Gravity = 2000.f;
}

void ASkatePawn::BeginPlay()
{
	Super::BeginPlay();
}

void ASkatePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Forward = GetActorForwardVector();
	FVector NewLocation = GetActorLocation() + (Forward * CurrentSpeed * DeltaTime);

	if (!bIsGrounded)
	{
		VerticalVelocity -= Gravity * DeltaTime;
		NewLocation.Z += VerticalVelocity * DeltaTime;

		if (!bIsGrounded)
		{
			TimeSinceJump += DeltaTime;
		}
		else
		{
			TimeSinceJump = 0.f;
		}
	}
	else
	{
		VerticalVelocity = 0.f;
	}

	SetActorLocation(NewLocation, true);

	ApplyFriction(DeltaTime);
	CheckGround();
	AlignToGround();
	ApplyAnimations();
}

void ASkatePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("TurnRight", this, &ASkatePawn::TurnRight);
	PlayerInputComponent->BindAction("Push", IE_Pressed, this, &ASkatePawn::Push);
	PlayerInputComponent->BindAction("Brake", IE_Pressed, this, &ASkatePawn::StartBrake);
	PlayerInputComponent->BindAction("Brake", IE_Released, this, &ASkatePawn::StopBrake);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASkatePawn::Jump);
}

void ASkatePawn::TurnRight(float Value)
{
	TurnInputValue = Value;

	if (FMath::IsNearlyZero(Value) || FMath::IsNearlyZero(CurrentSpeed))
	{
		return;
	}

	if (TurnTime < MaxTurnTime)
	{
		TurnTime += GetWorld()->GetDeltaSeconds();
		TurnTime = FMath::Clamp(TurnTime, 0.f, MaxTurnTime);
	}

	float TurnSpeed = FMath::Clamp(CurrentSpeed / MaxSpeed, 0.f, 1.f);
	float TurnRate = 100.f;
	float YawDelta = Value * TurnRate * TurnSpeed * GetWorld()->GetDeltaSeconds();
	AddActorWorldRotation(FRotator(0.f, YawDelta, 0.f));
}

void ASkatePawn::Push()
{
	CurrentSpeed = FMath::Clamp(CurrentSpeed + AccelerationRate * GetWorld()->GetDeltaSeconds(), 0.f, MaxSpeed);
}

void ASkatePawn::StartBrake()
{
	bIsBraking = true;
}

void ASkatePawn::StopBrake()
{
	bIsBraking = false;
}

void ASkatePawn::Jump()
{
	if (bIsGrounded)
	{
		VerticalVelocity = JumpStrength;
		bIsGrounded = false;
		TimeSinceJump = 0.f; 
	}
}

void ASkatePawn::ApplyAnimations()
{
	if (!FMath::IsNearlyZero(TurnInputValue) && TurnLeanCurve)
	{
		FVector RawLean = TurnLeanCurve->GetVectorValue(TurnTime);
		FVector Lean = RawLean * FMath::Sign(TurnInputValue); 

		FRotator MeshRot = FRotator(Lean.Y, MeshNeutralRotation.Yaw, Lean.X);
		SkateMesh->SetRelativeRotation(MeshRot);
	}
	else
	{
		TurnTime = 0.f;
		SkateMesh->SetRelativeRotation(FMath::RInterpTo(SkateMesh->GetRelativeRotation(), MeshNeutralRotation, GetWorld()->GetDeltaSeconds(), 5.f));
	}
}

void ASkatePawn::ApplyFriction(float DeltaTime)
{
	float AppliedDeceleration = bIsBraking ? BrakeStrength : DecelerationRate * 0.5f;

	if (CurrentSpeed > 0.f)
	{
		CurrentSpeed -= AppliedDeceleration * DeltaTime;
		if (CurrentSpeed < 5.f)
			CurrentSpeed = 0.f;
	}
}
void ASkatePawn::CheckGround()
{
	if (TimeSinceJump < GroundLockDuration)
	{
		bIsGrounded = false;
		return;
	}

	FHitResult Hit;
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.f, 0.f, 100.f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		float DistanceToGround = Start.Z - Hit.ImpactPoint.Z;
		const float GroundThreshold = 5.f;

		if (DistanceToGround <= GroundThreshold)
		{
			if (!bIsGrounded)
			{
				SetActorLocation(FVector(Start.X, Start.Y, Hit.ImpactPoint.Z));
			}
			bIsGrounded = true;
			VerticalVelocity = 0.f;
		}
		else
		{
			bIsGrounded = false;
		}
	}
	else
	{
		bIsGrounded = false;
	}
}
void ASkatePawn::AlignToGround()
{
	FVector ActorLocation = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();

	const float TraceOffset = 20.f;
	const float TraceDistance = 200.f;

	FVector FrontStart = ActorLocation + Forward * TraceOffset + FVector(0.f, 0.f, 50.f);
	FVector FrontEnd = FrontStart - FVector(0.f, 0.f, TraceDistance);

	FVector BackStart = ActorLocation - Forward * TraceOffset + FVector(0.f, 0.f, 50.f);
	FVector BackEnd = BackStart - FVector(0.f, 0.f, TraceDistance);

	FHitResult HitFront, HitBack;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bFrontHit = GetWorld()->LineTraceSingleByChannel(HitFront, FrontStart, FrontEnd, ECC_Visibility, Params);
	bool bBackHit = GetWorld()->LineTraceSingleByChannel(HitBack, BackStart, BackEnd, ECC_Visibility, Params);

//#if WITH_EDITOR
//	DrawDebugLine(GetWorld(), FrontStart, FrontEnd, FColor::Green, false, 0.f, 0, 1.f);
//	DrawDebugLine(GetWorld(), BackStart, BackEnd, FColor::Red, false, 0.f, 0, 1.f);
//#endif

	if (bFrontHit && bBackHit)
	{
		FVector SurfaceForward = (HitFront.ImpactPoint - HitBack.ImpactPoint).GetSafeNormal();
		FVector SurfaceNormal = FVector::CrossProduct(SurfaceForward, Right).GetSafeNormal();
		FRotator NewRotation = FRotationMatrix::MakeFromXZ(SurfaceForward, SurfaceNormal).Rotator();

		FVector AvgImpact = (HitFront.ImpactPoint + HitBack.ImpactPoint) / 2.f;
		FVector TargetLocation = FVector(ActorLocation.X, ActorLocation.Y, AvgImpact.Z + SnapHeight);
		SetActorLocation(FMath::VInterpTo(ActorLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), 10.f));

		FRotator CurrentRot = GetActorRotation();
		FRotator TargetRot = NewRotation;
		TargetRot.Roll = 0.f;
		SetActorRotation(FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), 10.f));
	}
}
