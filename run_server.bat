@echo off
title Basic Web Chat Server
echo ========================================
echo    Basic Web Chat Server - Runner
echo ========================================
echo.

echo Checking if server is built...
if not exist build\basic_web_chat.exe (
    echo ERROR: Server executable not found!
    echo.
    echo Please run build.bat first to compile the server
    echo.
    pause
    exit /b 1
)

echo [X] Server executable found
echo.

echo Checking database status...
if exist build\chat.db (
    echo [X] Database file exists (%CD%\build\chat.db)
    for %%F in (build\chat.db) do set DBSIZE=%%~zF
    if %DBSIZE% gtr 0 (
        echo [X] Database contains data (%DBSIZE% bytes)
    ) else (
        echo [!] Database file is empty
    )
) else (
    echo [!] No database file - new one will be created
)

echo.

echo Checking required DLLs...
if not exist build\sqlite3.dll (
    echo [!] sqlite3.dll not found in build directory
    echo     Server may fail to start
    echo     Try running build.bat again
)

echo.

echo ========================================
echo    BASIC VERSION FEATURES:
echo ========================================
echo 1. User registration/login (simple auth)
echo 2. Create chats (name only)
echo 3. Send/receive messages
echo 4. View your own chats only
echo.
echo MISSING FEATURES:
echo - Session tokens
echo - Chat search (search field is disabled)
echo - Public/private chats (all chats same)
echo - Adding users to chats
echo ========================================
echo.

echo Starting Basic Web Chat Server...
echo.
echo Server URL: http://localhost:8080
echo Database: %CD%\build\chat.db
echo.
echo Press Ctrl+C to stop the server
echo ========================================
echo.

cd build
basic_web_chat.exe

:: Если сервер завершился, возвращаемся в корневую папку
cd ..
echo.
echo Server stopped.
pause