@echo off
setlocal enabledelayedexpansion
chcp 65001
title [Development] 프로젝트 빌드

:: ============================
:: 프로젝트, 엔진 위치 등 빌드 관련 설정
:: ============================
set PROJECT_NAME=EternalDreams
set ENGINE_PATH="E:\Epic Games\UE_5.6_Custom\Engine"
set CONFIG=Development
set BUILD_PLATFORM=Win64
set PROJECT_PATH=%~dp0%PROJECT_NAME%.uproject
set UAT_PATH=%ENGINE_PATH%\Build\BatchFiles\RunUAT.bat
set BUILD_PATH=%ENGINE_PATH%\Build\BatchFiles\Build.bat

:: ============================
:: NuGet Audit 비활성화 설정
:: ============================
if not exist "E:\Epic Games\UE_5.6_Custom\\Directory.Build.props" (
    echo NuGet Audit 비활성화 설정 생성 중...
    (
        echo ^<Project^>
        echo   ^<PropertyGroup^>
        echo     ^<NuGetAudit^>false^</NuGetAudit^>
        echo     ^<NuGetAuditLevel^>none^</NuGetAuditLevel^>
        echo   ^</PropertyGroup^>
        echo ^</Project^>
    ) > "E:\Epic Games\UE_5.6_Custom\\Directory.Build.props"
    echo 생성 완료
)

echo ============================
echo 임시 파일 제거(Binaries, Intermediate)
if exist "%~dp0Binaries" rd /s /q "%~dp0Binaries"
if exist "%~dp0Intermediate" rd /s /q "%~dp0Intermediate"
echo 빌드 시작: %PROJECT_NAME%
echo ============================

echo 에디터 빌드 중...
call %BUILD_PATH% %PROJECT_NAME%Editor %BUILD_PLATFORM% %CONFIG% -project="%PROJECT_PATH%" -waitmutex
if errorlevel 1 (
    echo [오류] 에디터 빌드 실패!
    pause
    exit /b !ERRORLEVEL!
)

echo Client 빌드 중...
call %BUILD_PATH% %PROJECT_NAME%Client %BUILD_PLATFORM% %CONFIG% -project="%PROJECT_PATH%" -waitmutex
if errorlevel 1 (
    echo [오류] Client 빌드 실패!
    pause
    exit /b !ERRORLEVEL!
)

echo Server 빌드 중...
call %BUILD_PATH% %PROJECT_NAME%Server %BUILD_PLATFORM% %CONFIG% -project="%PROJECT_PATH%" -waitmutex
if errorlevel 1 (
    echo [오류] Server 빌드 실패!
    pause
    exit /b !ERRORLEVEL!
)

echo 컨텐츠 쿠킹 및 패키징 중...
call %UAT_PATH% BuildCookRun ^
    -project="%PROJECT_PATH%" ^
    -noP4 ^
    -clientconfig=%CONFIG% ^
    -serverconfig=%CONFIG% ^
    -platform=%BUILD_PLATFORM% ^
    -targetplatform=%BUILD_PLATFORM% ^
    -build ^
    -cook -allmaps -stage -pak -archive ^
    -archivedirectory="%~dp0BuildOutput\%CONFIG%" ^
    -client -server -compressed=false
if errorlevel 1 (
    echo 쿠킹 및 패키징 중 오류 발생! 코드=!ERRORLEVEL!
    pause
    exit /b !ERRORLEVEL!
)

echo ============================
echo 모든 작업 완료!
echo   출력: %~dp0BuildOutput\%CONFIG%
echo ============================
pause