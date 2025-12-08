@echo off
title Basic Web Chat Version 1.0 Tester
echo ========================================
echo    Basic Web Chat - Functional Tests
echo    Version 1.0 (Minimal Prototype)
echo ========================================
echo.

echo Project location: Desktop\naddya_v0.3
echo.

echo Step 1: Checking compiler...
g++ --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: g++ compiler not found!
    echo Please install MinGW and add it to PATH
    pause
    exit /b 1
)

echo [X] Compiler found
echo.

echo Step 2: Checking current directory...
echo Current: %CD%
echo.

echo Step 3: Going to project root (Desktop\naddya_v0.3)...
:: Возвращаемся в корень проекта
cd /d "C:\Users\%USERNAME%\Desktop\naddya_v0.3" 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Cannot find project on Desktop!
    echo Looking for naddya_v0.3 folder...
    dir "%USERPROFILE%\Desktop\"
    pause
    exit /b 1
)

echo Project root: %CD%
echo.

echo Step 4: Checking project structure...
if not exist "backend\tests\tester.cpp" (
    echo ERROR: tester.cpp not found!
    echo Looking in: %CD%\backend\tests\
    dir "backend\tests\"
    pause
    exit /b 1
)

if not exist "backend\src\chat_manager.cpp" (
    echo ERROR: chat_manager.cpp not found!
    echo Looking in: %CD%\backend\src\
    dir "backend\src\"
    pause
    exit /b 1
)

echo [X] All source files found
echo.

echo Step 5: Creating build directory for tests...
if not exist "build\tests" mkdir "build\tests"
cd "build\tests"

echo Step 6: Compiling test suite...
echo This may take a moment...
echo.

echo Compiling from project root:
echo - %CD%\..\..\backend\tests\tester.cpp
echo - %CD%\..\..\backend\src\*.cpp
echo.

g++ -std=c++17 -O2 ^
  -I"..\..\Crow\include" ^
  -I"..\..\asio\asio\include" ^
  -I"..\..\backend\src" ^
  "..\..\backend\tests\tester.cpp" ^
  "..\..\backend\src\chat_manager.cpp" ^
  "..\..\backend\src\user.cpp" ^
  "..\..\backend\src\chat.cpp" ^
  "..\..\backend\src\message.cpp" ^
  "..\..\backend\src\database.cpp" ^
  -lws2_32 -lwsock32 -lbcrypt -lsqlite3 ^
  -o test_basic.exe

if %errorlevel% neq 0 (
    echo.
    echo Standard compilation failed. Trying with local SQLite...
    echo.
    
    :: Попробуем с библиотекой из build
    if exist "..\libsqlite3.a" (
        echo Using libsqlite3.a from build directory
        g++ -std=c++17 -O2 ^
          -I"..\..\Crow\include" ^
          -I"..\..\asio\asio\include" ^
          -I"..\..\backend\src" ^
          "..\..\backend\tests\tester.cpp" ^
          "..\..\backend\src\chat_manager.cpp" ^
          "..\..\backend\src\user.cpp" ^
          "..\..\backend\src\chat.cpp" ^
          "..\..\backend\src\message.cpp" ^
          "..\..\backend\src\database.cpp" ^
          -lws2_32 -lwsock32 -lbcrypt "..\libsqlite3.a" ^
          -o test_basic.exe
    ) else (
        echo Looking for SQLite DLL...
        where sqlite3.dll
        
        echo.
        echo Trying without SQLite linking...
        g++ -std=c++17 -O2 ^
          -I"..\..\Crow\include" ^
          -I"..\..\asio\asio\include" ^
          -I"..\..\backend\src" ^
          "..\..\backend\tests\tester.cpp" ^
          "..\..\backend\src\chat_manager.cpp" ^
          "..\..\backend\src\user.cpp" ^
          "..\..\backend\src\chat.cpp" ^
          "..\..\backend\src\message.cpp" ^
          "..\..\backend\src\database.cpp" ^
          -lws2_32 -lwsock32 -lbcrypt ^
          -o test_basic.exe
    )
)

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Test compilation failed!
    echo.
    echo Solutions:
    echo 1. Make sure build.bat was run successfully
    echo 2. Check that SQLite is installed
    echo 3. Verify all source files exist
    echo.
    echo Press any key to show directory structure...
    pause
    
    echo.
    echo Directory structure:
    echo =====================
    dir "..\..\"
    echo.
    dir "..\..\backend\"
    echo.
    dir "..\..\backend\tests\"
    echo.
    dir "..\..\backend\src\"
    
    pause
    exit /b 1
)

echo.
echo [X] Test suite compiled successfully!
echo.

echo Step 7: Copying required DLLs...
if exist "..\sqlite3.dll" (
    copy "..\sqlite3.dll" . >nul 2>&1
    echo [X] Copied sqlite3.dll
) else (
    echo [!] sqlite3.dll not found in build directory
    echo Test may need DLL in PATH
)

echo.

echo Step 8: Running tests...
echo ========================================
echo.
test_basic.exe

set TEST_RESULT=%errorlevel%

echo.
echo ========================================

if %TEST_RESULT% equ 0 (
    echo.
    echo ALL TESTS PASSED!
    echo.
    echo Basic Version 1.0 functionality verified:
    echo User registration/login
    echo Chat creation
    echo Message sending/receiving
    echo User chats list
    echo Database persistence
    echo Error handling
    echo.
) else (
    echo.
    echo SOME TESTS FAILED!
    echo Check the output above for details
    echo.
)

echo Step 9: Cleaning up test database...
del test_chat.db >nul 2>&1

echo.
echo ========================================
echo Test execution completed.
echo Press any key to exit...
pause >nul

:: Возвращаемся в директорию тестов
cd /d "C:\Users\%USERNAME%\Desktop\naddya_v0.3\backend\tests"