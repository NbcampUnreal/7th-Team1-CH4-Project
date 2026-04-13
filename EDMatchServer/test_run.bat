@echo off
echo ============================================================
echo  EDMatchServer E2E Test
echo  Prerequisites: Redis, MySQL running
echo  PLAYERS_PER_MATCH = 2 (test mode)
echo ============================================================
echo.
echo [Step 1] Start IOCP Server (press Ctrl+C to stop)
echo   In Terminal 1:  build3\Release\EDMatchServer.exe
echo.
echo [Step 2] Start Dedi Simulator
echo   In Terminal 2:  python test_client.py --dedi 127.0.0.1 7777
echo.
echo [Step 3] Start Client 1 (auto login + queue + ready)
echo   In Terminal 3:  python test_client.py --auto test1 1234
echo.
echo [Step 4] Start Client 2 (auto login + queue + ready)
echo   In Terminal 4:  python test_client.py --auto test2 1234
echo.
echo [Expected Result]
echo   - Both clients login successfully
echo   - Both join matchmaking queue
echo   - Match found (2 players)
echo   - Both auto-ready
echo   - Dedi simulator receives DEDI_ASSIGN_MATCH
echo   - Both clients receive GAME_START with dedi IP:port
echo.
echo ============================================================
echo  Interactive Mode:
echo   python test_client.py
echo   Commands: login, queue, cancel, ready, team, dedi, quit
echo ============================================================
pause
