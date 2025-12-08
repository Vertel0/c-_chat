@echo off
title Web Chat v2.0 Tester
echo ========================================
echo    Web Chat v2.0 Functional Tests
echo ========================================
echo    Testing Search and Join functionality
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

echo Step 3: Compiling v2.0 test suite...
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
  -o test_v2.exe

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
          -o test_v2.exe
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

echo Step 4: Running v2.0 tests...
echo ========================================
echo.
test_v2.exe

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo.
    echo ALL v2.0 TESTS PASSED!
    echo.
    echo Version 2.0 features verified:
    echo  User registration and login
    echo  Chat creation
    echo  Message sending and receiving
    echo  User chats list
    echo  SEARCH functionality 
    echo  JOIN functionality 
    echo  Multi-user participation 
    echo  Database persistence
    echo.
    set TEST_RESULT=0
) else (
    echo.
    echo ========================================
    echo.
    echo SOME TESTS FAILED!
    echo Check the output above for details
    echo.
    set TEST_RESULT=1
)

echo.
echo ========================================
echo v2.0 Test execution completed.
echo.
echo Next: Run run_server_v2.bat to test manually

if %TEST_RESULT% equ 0 (
    echo.
    echo Press any key to exit...
) else (
    echo.
    echo Press any key to exit...
)
pause >nul

echo Step 5: Cleaning up...
del test_chat_v2.db >nul 2>&1

echo.
echo ========================================
echo v2.0 Test execution completed.
echo.
echo Next: Run run_server_v2.bat to test manually
echo Press any key to exit...
pause >nul

cd ..\..