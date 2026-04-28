# CombatPortfolio Development Log

## Project Goal

C++ 중심의 3인칭 액션 전투 포트폴리오를 제작한다.  
빈 Unreal Engine 프로젝트에서 시작하여 이동, 카메라, 락온, 회피, 콤보, 피격, AI, UI, 디버그 기능까지 직접 구현한다.

## Technical Direction

- Blank C++ Project 기반
- Enhanced Input 기반 입력 시스템
- C++로 핵심 로직 구현
- Blueprint는 데이터 설정과 연출 레이어로 제한적으로 사용
- 기능별 폴더 구조와 컴포넌트 기반 설계 적용
- Git + Git LFS를 사용한 버전 관리

## Episode 1

### Completed

- Blank C++ Unreal project created
- Base Content folder structure created
- Enhanced Input plugin checked
- EnhancedInput module dependency added
- Git repository initialized
- Git LFS configured

### Notes

템플릿 프로젝트를 사용하지 않고 직접 구조를 구성함으로써 입력, 캐릭터, 카메라, 게임모드의 동작 원리를 단계적으로 학습할 수 있도록 했다.





## Episode 2

### Goal

Blank C++ 프로젝트에서 직접 플레이어 캐릭터, 플레이어 컨트롤러, 게임모드를 구성하고 Enhanced Input 기반 이동/카메라 입력을 구현한다.

### Completed

- Created `ACombatPlayerCharacter`
- Added SpringArm and Camera components in C++
- Configured basic CharacterMovement settings
- Created `ACombatPlayerController`
- Registered Enhanced Input Mapping Context through PlayerController
- Created `ACombatGameMode`
- Created `IA_Move`, `IA_Look`, and `IMC_Player`
- Created Blueprint subclasses for Character, PlayerController, and GameMode
- Set default pawn and controller through GameMode
- Verified WASD movement and mouse look in `L_CombatSandbox`

### Technical Notes

- Character is used as the player body because it provides capsule collision, mesh support, and built-in movement logic.
- PlayerController owns the input mapping registration because it represents the local player's control layer.
- Core movement logic is implemented in C++, while asset references are assigned in Blueprint.