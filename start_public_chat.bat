@echo off
title Public Web Chat v2.0 with Ngrok
echo ========================================
echo    Public Web Chat v2.0 - Ngrok Edition
echo ========================================
echo.

echo Step 1: Checking if server is built...
if not exist build\web_chat_v2.exe (
    echo Server not found! Running build.bat...
    call build.bat
    if %errorlevel% neq 0 (
        echo Build failed!
        pause
        exit /b 1
    )
)

echo [X] Server is ready
echo.

echo Step 2: Starting local server in background...
echo Starting Web Chat Server v2.0 on port 8080...
start "Web Chat Server v2.0" /B cmd /c "cd build && web_chat_v2.exe"
echo [X] Local server starting...
timeout /t 5 /nobreak >nul

echo Step 3: Starting ngrok tunnel...
echo.
echo ========================================
echo    🎉 YOUR CHAT v2.0 IS NOW PUBLIC!
echo ========================================
echo.
echo Waiting for ngrok to start...
timeout /t 2 /nobreak >nul

echo Starting ngrok tunnel...
ngrok http 8080