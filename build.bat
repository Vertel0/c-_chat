@echo off
title Web Chat Server Builder
echo ========================================
echo    Web Chat Server - Build Script
echo ========================================
echo.

echo Step 1: Checking MinGW compiler...
g++ --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: g++ compiler not found!
    echo Please install MinGW and add it to PATH
    pause
    exit /b 1
)

echo [X] MinGW compiler found
echo.

echo Step 2: Checking SQLite3...
set SQLITE_FOUND=0

if exist "C:\msys64\ucrt64\bin\sqlite3.dll" (
    echo [X] Found SQLite3 in MSYS2
    set SQLITE_LIB=C:\msys64\ucrt64\lib\libsqlite3.a
    set SQLITE_FOUND=1
) else if exist "C:\MinGW\bin\sqlite3.dll" (
    echo [X] Found SQLite3 in MinGW
    set SQLITE_LIB=C:\MinGW\lib\libsqlite3.a
    set SQLITE_FOUND=1
) else if exist "C:\Windows\System32\sqlite3.dll" (
    echo [X] Found SQLite3 in System32
    set SQLITE_FOUND=1
) else (
    where sqlite3.dll >nul 2>&1
    if %errorlevel% equ 0 (
        echo [X] Found sqlite3.dll in PATH
        set SQLITE_FOUND=1
    )
)

if %SQLITE_FOUND% equ 0 (
    echo WARNING: SQLite3 not found in standard locations!
    echo Attempting to download...
    call download_sqlite.bat
    if %errorlevel% neq 0 (
        echo ERROR: Failed to setup SQLite3
        pause
        exit /b 1
    )
)

echo.

echo Step 3: Creating build directory...
if not exist build mkdir build
cd build

echo Step 4: Preserving existing database...
if exist chat.db (
    echo [X] Found existing database, creating backup...
    copy chat.db chat.db.backup >nul 2>&1
    set DB_BACKUP=1
) else (
    set DB_BACKUP=0
)

echo Step 5: Checking file structure...
if not exist "..\Crow\include\crow.h" (
    echo ERROR: ..\Crow\include\crow.h not found!
    pause
    exit /b 1
)

if not exist "..\Crow\include\crow\common.h" (
    echo ERROR: ..\Crow\include\crow\common.h not found!
    pause
    exit /b 1
)

if not exist "..\asio\asio\include\asio.hpp" (
    echo ERROR: ..\asio\asio\include\asio.hpp not found!
    pause
    exit /b 1
)

if not exist "..\backend\src\database.h" (
    echo ERROR: ..\backend\src\database.h not found!
    pause
    exit /b 1
)

if not exist "..\backend\src\database.cpp" (
    echo ERROR: ..\backend\src\database.cpp not found!
    pause
    exit /b 1
)

echo [X] All source files found
echo.

echo Step 6: Compiling server...
echo This may take a minute...
echo.

:: Попытка 1: с -lsqlite3
echo Attempt 1: Standard linking...
g++ -std=c++17 -O2 -pthread ^
  -I"../Crow/include" ^
  -I"../asio/asio/include" ^
  -I"../backend/src" ^
  "../backend/src/main.cpp" ^
  "../backend/src/webserver.cpp" ^
  "../backend/src/chat_manager.cpp" ^
  "../backend/src/user.cpp" ^
  "../backend/src/chat.cpp" ^
  "../backend/src/message.cpp" ^
  "../backend/src/database.cpp" ^
  -lws2_32 -lwsock32 -lbcrypt -lsqlite3 ^
  -o web_chat_server.exe

if %errorlevel% neq 0 (
    echo.
    echo Attempt 1 failed. Trying Attempt 2: Direct library...
    
    if exist "%SQLITE_LIB%" (
        g++ -std=c++17 -O2 -pthread ^
          -I"../Crow/include" ^
          -I"../asio/asio/include" ^
          -I"../backend/src" ^
          "../backend/src/main.cpp" ^
          "../backend/src/webserver.cpp" ^
          "../backend/src/chat_manager.cpp" ^
          "../backend/src/user.cpp" ^
          "../backend/src/chat.cpp" ^
          "../backend/src/message.cpp" ^
          "../backend/src/database.cpp" ^
          -lws2_32 -lwsock32 -lbcrypt "%SQLITE_LIB%" ^
          -o web_chat_server.exe
    ) else if exist "libsqlite3.a" (
        echo Using downloaded SQLite library...
        g++ -std=c++17 -O2 -pthread ^
          -I"../Crow/include" ^
          -I"../asio/asio/include" ^
          -I"../backend/src" ^
          "../backend/src/main.cpp" ^
          "../backend/src/webserver.cpp" ^
          "../backend/src/chat_manager.cpp" ^
          "../backend/src/user.cpp" ^
          "../backend/src/chat.cpp" ^
          "../backend/src/message.cpp" ^
          "../backend/src/database.cpp" ^
          -lws2_32 -lwsock32 -lbcrypt "libsqlite3.a" ^
          -o web_chat_server.exe
    ) else (
        echo No SQLite library found, trying without...
        g++ -std=c++17 -O2 -pthread ^
          -I"../Crow/include" ^
          -I"../asio/asio/include" ^
          -I"../backend/src" ^
          "../backend/src/main.cpp" ^
          "../backend/src/webserver.cpp" ^
          "../backend/src/chat_manager.cpp" ^
          "../backend/src/user.cpp" ^
          "../backend/src/chat.cpp" ^
          "../backend/src/message.cpp" ^
          "../backend/src/database.cpp" ^
          -lws2_32 -lwsock32 -lbcrypt ^
          -o web_chat_server.exe
    )
)

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Compilation failed!
    echo.
    echo Troubleshooting tips:
    echo 1. Make sure all required files are in place
    echo 2. Check that SQLite3 is properly installed
    echo 3. Try running the download_sqlite.bat manually
    pause
    exit /b 1
)

echo.
echo [X] Compilation successful!
echo.

echo Step 7: Setting up files...
if %DB_BACKUP% equ 1 (
    echo Restoring database from backup...
    if exist chat.db.backup (
        move chat.db.backup chat.db >nul 2>&1
        echo [X] Database restored
    )
)

xcopy "..\backend\templates" "templates" /E /I /Y >nul
xcopy "..\backend\static" "static" /E /I /Y >nul

:: Копируем sqlite3.dll если нужно
if not exist sqlite3.dll (
    if exist "C:\msys64\ucrt64\bin\sqlite3.dll" (
        copy "C:\msys64\ucrt64\bin\sqlite3.dll" . >nul 2>&1
        echo [X] Copied sqlite3.dll from MSYS2
    ) else if exist "C:\MinGW\bin\sqlite3.dll" (
        copy "C:\MinGW\bin\sqlite3.dll" . >nul 2>&1
        echo [X] Copied sqlite3.dll from MinGW
    ) else if exist "C:\Windows\System32\sqlite3.dll" (
        copy "C:\Windows\System32\sqlite3.dll" . >nul 2>&1
        echo [X] Copied sqlite3.dll from System32
    ) else if exist "..\sqlite3.dll" (
        copy "..\sqlite3.dll" . >nul 2>&1
        echo [X] Copied sqlite3.dll from project root
    ) else (
        echo [!] sqlite3.dll not found - server may not start
    )
)

echo.
echo ========================================
echo    BUILD COMPLETED SUCCESSFULLY! 🎉
echo ========================================
echo.
echo Next steps:
echo 1. Use run_server.bat to start the server
echo 2. Open http://localhost:8080 in your browser
echo.
echo Files created:
echo - web_chat_server.exe (main server)
echo - chat.db (database file)
echo - templates/ (HTML templates)
echo - static/ (CSS/JS files)
echo.
if %DB_BACKUP% equ 1 (
    echo [X] Existing database was preserved
) else (
    echo [!] New database will be created on first run
)
echo.
echo Press any key to return to main directory...
pause >nul

cd ..