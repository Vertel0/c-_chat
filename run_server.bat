@echo off
title Web Chat Server
echo ========================================
echo    Web Chat Server - Runner
echo ========================================
echo.

echo Checking if server is built...
if not exist build\web_chat_server.exe (
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

echo Starting Web Chat Server...
echo.
echo Server URL: http://localhost:8080
echo Database: %CD%\build\chat.db
echo.
echo Press Ctrl+C to stop the server
echo ========================================
echo.

cd build
web_chat_server.exe

:: Если сервер завершился, возвращаемся в корневую папку
cd ..
echo.
echo Server stopped.
pause