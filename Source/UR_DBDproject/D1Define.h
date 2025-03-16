#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "D1Define.generated.h"

#define D(x) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, x); }

UENUM(BlueprintType)
enum class ECreatureState : uint8
{
	None,
	Idle,
	Walk,
	Run,
	Crouch,
	Attack1,
	Skill1,
	InTransform,
	Interactable,
	Dead,
	Parkour,
};

UENUM(BlueprintType)
enum class ESurvivorState : uint8
{
	None,
	Healthy     UMETA(DisplayName = "Healthy"),   // 건강 상태 (기본)
	Injured     UMETA(DisplayName = "Injured"),   // 부상 상태 (살인마 공격 1회)
	Crawl		UMETA(DisplayName = "Crawl"),     // 기절 상태 (살인마 공격 2회)
	PickedUp	UMETA(DisplayName = "Picked Up"), // 킬러가 들고 있는 상태
	Hooked		UMETA(DisplayName = "Hooked"),    // 갈고리 상태 (갈고리에 걸림)
	Dying		UMETA(DisplayName = "Dying"),     // 사망 상태
};

UENUM(BlueprintType)
enum class EGeneratorInteractionPosition : uint8
{
	None,
	Front UMETA(DisplayName = "Front"),
	Back UMETA(DisplayName = "Back"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class EVaultType : uint8
{
	Slow UMETA(DisplayName = "Slow"),
	Medium UMETA(DisplayName = "Medium"),
	Fast UMETA(DisplayName = "Fast")
};

UENUM(BlueprintType)
enum class EVaultInteractionPosition : uint8
{
	Front UMETA(DisplayName = "Front"),
	Back UMETA(DisplayName = "Back"),
};

UENUM(BlueprintType)
enum class EDraculaTransformationState : uint8
{
	Dracula UMETA(DisplayName = "Dracula"),
	Wolf UMETA(DisplayName = "Wolf"),
	Bat UMETA(DisplayName = "Bat"), // 변신 선택 모드
};

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	// 열거형 이름이랑 DisplayeName이랑 같아야 함

	/*
		Survivor
	*/
	NONE UMETA(DisplayName = "NONE"),
	MEG UMETA(DisplayName = "MEG"),

	/*
		Killer
	*/
	DRACULA UMETA(DisplayName = "DRACULA"),
};

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString userIP;

	UPROPERTY(BlueprintReadWrite)
	ECharacterType characterType;

	FPlayerInfo() : userIP("127.0.0.1"), characterType(ECharacterType::NONE) {}
};

USTRUCT(BlueprintType)
struct FServerInfo // 리슨 서버장 ( 킬러 ) 소켓 서버에서 받아야 할 정보
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FPlayerInfo> survivorInfos;

	UPROPERTY(BlueprintReadWrite)
	FPlayerInfo killerInfo;

	UPROPERTY(BlueprintReadWrite)
	bool bIsServer = false; // 서버장 체크
};

USTRUCT(BlueprintType)
struct FCharacterDataSet : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APlayerController> ControllerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APlayerState> PlayerStateClass;
};