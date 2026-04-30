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




## Episode 3

### Goal

플레이어 캐릭터에 Skeletal Mesh와 Animation Blueprint를 연결하고, 이동 속도 기반 Locomotion Blend Space를 구성한다.

### Completed

- Added temporary mannequin skeletal mesh for player visualization
- Created `UCombatAnimInstance`
- Calculated `GroundSpeed` and `bIsFalling` in C++
- Created `ABP_CombatPlayer` based on `UCombatAnimInstance`
- Created `BS_Player_Locomotion`
- Connected Idle / Walk / Run animations through Blend Space
- Assigned Animation Blueprint to `BP_CombatPlayerCharacter`
- Verified movement animation in `L_CombatSandbox`

### Technical Notes

- Character capsule is used as the collision and movement root.
- Skeletal Mesh is used only for visual representation.
- Animation state values are calculated in C++ and consumed by Animation Blueprint.
- Basic locomotion uses in-place animation driven by `CharacterMovement`.



## Episode 4

### Goal

플레이어 캐릭터의 이동 상태를 걷기, 기본 달리기, 스프린트로 분리하고, 입력 기반 상태 관리 구조를 구현한다.

### Completed

- Added `IA_Walk` and `IA_Sprint`
- Mapped `Left Ctrl` to walk and `Left Shift` to sprint
- Added `ECombatMovementState`
- Added movement speed variables: `WalkSpeed`, `RunSpeed`, `SprintSpeed`
- Implemented movement state update flow
- Applied movement speed through `CharacterMovementComponent::MaxWalkSpeed`
- Added camera sensitivity variables
- Added movement debug output using `GEngine->AddOnScreenDebugMessage`
- Updated locomotion Blend Space axis from 500 to 700

### Technical Notes

- Input intent and final movement state are separated.
- `bWantsToWalk` and `bWantsToSprint` represent player input intent.
- `MovementState` represents the resolved movement state after priority rules.
- Movement speed is updated through a single function to keep the logic centralized.
- Debug output is currently Tick-based and will later be moved behind a dedicated debug system.



## Episode 5

### Goal

전투 게임에 필요한 캐릭터 회전 기준을 정리하고, 일반 이동 모드와 Strafe 모드를 전환할 수 있는 구조를 구현한다.

### Completed

- Added `IA_ToggleRotationMode`
- Mapped `Tab` to rotation mode toggle
- Added `ECombatRotationMode`
- Implemented `OrientToMovement` rotation mode
- Implemented `Strafe` rotation mode
- Added `SetRotationMode()` and `ApplyRotationMode()`
- Added camera lag to SpringArm
- Added camera pitch limits in `ACombatPlayerController`
- Extended debug output to show current rotation mode and control yaw

### Technical Notes

- `OrientToMovement` mode rotates the character toward movement direction.
- `Strafe` mode rotates the character using Controller Yaw.
- Rotation mode is separated from movement state to prepare for lock-on combat.
- Camera pitch limits are controlled by PlayerController through PlayerCameraManager.