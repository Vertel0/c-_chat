@echo off
title Web Chat Server v2.0
echo ========================================
echo    Web Chat Server v2.0 - Runner
echo ========================================
echo    (With Search and Join functionality)
echo ========================================
echo.

echo Checking if server is built...
if not exist build\web_chat_v2.exe (
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
    for %%F in (build\chat.db) do (
        set DBSIZE=%%~zF
    )
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
echo    VERSION 2.0 FEATURES:
echo ========================================
echo 1. User registration and login (simple auth)
echo 2. Create chats (name only)
echo 3. Send and receive messages
echo 4. View your own chats
echo 5. SEARCH chats by ID ✓ (NEW!)
echo 6. JOIN any public chat ✓ (NEW!)
echo.
echo ========================================
echo.

echo Starting Web Chat Server v2.0...
echo.
echo Server URL: http://localhost:8080
echo Database: %CD%\build\chat.db
echo.
echo Press Ctrl+C to stop the server
echo ========================================
echo.

cd build
web_chat_v2.exe

:: Если сервер завершился, возвращаемся в корневую папку
cd ..
echo.
echo Server stopped.
pause