# UR_DBD: Unreal Engine "Dead by Daylight" Project

## 프로젝트 소개 (Introduction)

이 프로젝트는 인기 비대칭 서바이벌 호러 게임 **'Dead by Daylight'**를 **Unreal Engine 5.5**를 사용하여 재해석하고 구현한 것입니다.

핵심 목표는 원작의 게임플레이 메카닉을 깊이 있게 학습하고, 특히 언리얼 엔진의 **게임플레이 어빌리티 시스템 (Gameplay Ability System, GAS)**을 활용하여 캐릭터의 복잡한 스킬, 아이템, 상태 효과를 체계적으로 구현하는 것입니다.

---

## 📹 데모
- Dead by Daylight 모작: https://www.youtube.com/watch?v=YTgI7ov183M

---

## 🚀 엔진 및 주요 모듈 (Engine & Core Modules)

- **Engine Version**: `Unreal Engine 5.5`
- **Core Modules**:
  - `GameplayAbilities`: 캐릭터의 모든 능력과 상호작용을 관리하는 핵심 시스템입니다.
  - `UMG`: 게임의 HUD, 메뉴 등 사용자 인터페이스를 제작합니다.

## 📌 핵심 기능 (Core Features)

- **비대칭 게임플레이 (Asymmetrical Gameplay)**: 생존자(Survivor)와 살인마(Killer)의 역할에 따라 구분되는 고유한 능력과 게임 목표를 구현합니다.
- **게임플레이 어빌리티 시스템 (GAS)**: 캐릭터의 퍽, 애드온, 아이템 효과, 상태 이상 등 모든 어빌리티를 GAS를 통해 관리하여 확장성과 재사용성을 높였습니다.
- **상호작용 시스템 (Interactable System)**: 발전기, 창틀, 판자 등 'Dead by Daylight'의 핵심적인 환경 오브젝트와의 상호작용을 구현합니다.
- **캐릭터 및 애니메이션 (Character & Animation)**: GAS와 연동되는 캐릭터 클래스와 커스텀 애니메이션 시스템을 구현합니다.

## 📂 소스 코드 구조 (Source Code Structure)

```
Source/UR_DBDproject/
├── AbilitySystem/      	# GAS 관련 클래스 (어빌리티, 어트리뷰트, 이펙트 등)
├── Animation/          	# 애니메이션 블루프린트 및 커스텀 노티파이
├── Characters/         	# 생존자, 살인마 등 기본 캐릭터 클래스
├── GameMode/        	# 게임의 규칙과 승리/패배 조건을 정의하는 게임 모드
├── Interactables/      	# 발전기, 판자 등 상호작용 가능한 액터
├── Items/               	# 캐릭터가 사용하는 아이템
├── System/             	# 게임의 핵심 시스템 및 관리자 클래스
└── UI/                 	# 위젯 블루프린트 등 UI 관련 코드
```

## 🛠 빌드 (Build)

1) 이 저장소를 로컬 컴퓨터에 클론합니다.
2) `UR_DBDproject.uproject` 파일을 우클릭한 후 **'Generate Visual Studio project files'**를 선택하여 Visual Studio 솔루션 파일을 생성합니다.
3) 생성된 `.sln` 파일을 Visual Studio에서 엽니다.
4) 솔루션을 빌드합니다.
5) Unreal Editor에서 `UR_DBDproject.uproject` 파일을 열고 플레이(Play) 버튼을 눌러 게임을 실행합니다.
