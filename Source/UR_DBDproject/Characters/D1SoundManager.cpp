// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/D1SoundManager.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AD1SoundManager::AD1SoundManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    BGMComponentA = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponentA"));
    BGMComponentA->bAutoActivate = false;
    BGMComponentA->bIsUISound = false;
    RootComponent = BGMComponentA;

    BGMComponentB = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponentB"));
    BGMComponentB->bAutoActivate = false;
    BGMComponentB->bIsUISound = false;
    RootComponent = BGMComponentB;

    CurrentComponent = BGMComponentA;
    PreviousComponent = BGMComponentB;

    SFXComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SFXComponent"));
    SFXComponent->bAutoActivate = false;
    SFXComponent->bIsUISound = false;
    SFXComponent->SetupAttachment(RootComponent);

    bIsFadingOut = false;
    CurrentBGM = nullptr;
}

// Called when the game starts or when spawned
void AD1SoundManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AD1SoundManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AD1SoundManager::PlayBGM(USoundBase* Music, float FadeTime)
{
    if (!Music || Music == CurrentBGM || bIsFadingOut)
        return;

    UAudioComponent* Temp = CurrentComponent;
    CurrentComponent = PreviousComponent;
    PreviousComponent = Temp;

    // 새 트랙 설정 및 재생
    CurrentComponent->SetSound(Music);
    CurrentComponent->FadeIn(FadeTime, 1.0f, 0.0f, EAudioFaderCurve::SCurve);
    CurrentBGM = Music;

    // 이전 트랙 페이드아웃
    if (PreviousComponent->IsPlaying())
    {
        PreviousComponent->FadeOut(0.5f, 0.0f);
    }
}

void AD1SoundManager::StopBGM(float FadeTime)
{
    if (CurrentComponent->IsPlaying())
    {
        CurrentComponent->FadeOut(FadeTime, 0.0f, EAudioFaderCurve::Sin);
        CurrentBGM = nullptr;
    }
}

void AD1SoundManager::PlaySFX(USoundBase* Sound)
{
    if (Sound && SFXComponent)
    {
        SFXComponent->SetSound(Sound);
        SFXComponent->Play();
    }
}
