@echo off
title Web Chat Tester
echo ========================================
echo    Web Chat Functional Tests
echo ========================================
echo    Running ALL tests
echo ========================================
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

echo Step 2: Creating test directory...
if not exist build\tests mkdir build\tests
cd build\tests

echo Step 3: Compiling test suite...
echo.
echo Compiling tester.cpp...
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
  -o tester.exe

if %errorlevel% neq 0 (
    echo.
    echo Compilation failed. Trying alternative...
    
    if exist "..\libsqlite3.a" (
        g++ -std=c++17 -O2 ^
          -I"..\..\Crow\include" ^
          -I"..\..\asio\asio/include" ^
          -I"..\..\backend\src" ^
          "..\..\backend\tests\tester.cpp" ^
          "..\..\backend\src\chat_manager.cpp" ^
          "..\..\backend\src\user.cpp" ^
          "..\..\backend\src\chat.cpp" ^
          "..\..\backend\src\message.cpp" ^
          "..\..\backend\src\database.cpp" ^
          -lws2_32 -lwsock32 -lbcrypt "..\libsqlite3.a" ^
          -o tester.exe
    ) else (
        echo ERROR: Cannot compile tests
        pause
        exit /b 1
    )
)

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Test compilation failed!
    pause
    exit /b 1
)

echo.
echo [X] Test suite compiled successfully!
echo.

echo Step 4: Running Version 2.0 tests...
echo ========================================
echo.
tester.exe 2

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Version 2.0 tests PASSED!
    echo.
    echo Version 2.0 features verified:
    echo - User registration and login
    echo - Chat creation
    echo - Message sending and receiving
    echo - User chats list
    echo - SEARCH functionality 
    echo - JOIN functionality 
    echo - Multi-user participation 
    echo - Database persistence
    echo.
    set V2_RESULT=0
) else (
    echo.
    echo ========================================
    echo Version 2.0 tests FAILED!
    echo.
    set V2_RESULT=1
)

echo.
echo Step 5: Running Version 3.0 tests...
echo ========================================
echo.
tester.exe 3

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Version 3.0 tests PASSED!
    echo.
    echo Version 3.0 features verified:
    echo - User registration and login
    echo - Chat creation (public/private)
    echo - Message sending and receiving
    echo - User chats list
    echo - Search functionality 
    echo - Public chat join 
    echo - Private chat access control 
    echo - Invite functionality 
    echo - Whitelist management 
    echo - Database persistence
    echo.
    set V3_RESULT=0
) else (
    echo.
    echo ========================================
    echo Version 3.0 tests FAILED!
    echo.
    set V3_RESULT=1
)

echo.
echo Step 6: Test summary...
echo ========================================
if %V2_RESULT% equ 0 (
    echo Version 2.0: PASSED
) else (
    echo Version 2.0: FAILED
)

if %V3_RESULT% equ 0 (
    echo Version 3.0: PASSED
) else (
    echo Version 3.0: FAILED
)

if %V2_RESULT% equ 0 if %V3_RESULT% equ 0 (
    echo.
    echo ALL TESTS PASSED!
    set TEST_RESULT=0
) else (
    echo.
    echo SOME TESTS FAILED!
    set TEST_RESULT=1
)

echo.
echo Step 7: Cleaning up...
del test_chat_v2.db >nul 2>&1
del test_chat_v3.db >nul 2>&1

echo.
echo ========================================
echo All tests execution completed.
echo.
echo Next: Run the server and test manually
echo Press any key to exit...
pause >nul

cd ..\..