"""
EDMatchServer 테스트 클라이언트
사용법:
  python test_client.py                    # 대화형 모드
  python test_client.py --auto <id> <pw>   # 자동 모드 (로그인→매칭→레디)
  python test_client.py --dedi <ip> <port> # 데디서버 시뮬레이션
"""

import socket
import struct
import json
import sys
import threading
import time

# ============================================================
#  Protocol
# ============================================================

HEADER_SIZE = 6  # 4(length) + 2(opcode)

# Client -> Server
C2S_LOGIN_REQ       = 0x0001
C2S_MATCH_QUEUE_REQ = 0x0010
C2S_MATCH_CANCEL    = 0x0040
C2S_LOBBY_READY     = 0x0020
C2S_LOBBY_TEAM_CHANGE = 0x0021

# Server -> Client
S2C_LOGIN_RES       = 0x0002
S2C_MATCH_QUEUE_RES = 0x0011
S2C_MATCH_FOUND     = 0x0012
S2C_LOBBY_STATE     = 0x0022
S2C_GAME_START      = 0x0030

# DediServer -> Server
D2S_DEDI_REGISTER    = 0x1001
D2S_DEDI_HEARTBEAT   = 0x1003
D2S_DEDI_MATCH_RESULT = 0x1004

# Server -> DediServer
S2D_DEDI_ASSIGN      = 0x1002

OPCODE_NAMES = {
    S2C_LOGIN_RES: "LOGIN_RES",
    S2C_MATCH_QUEUE_RES: "MATCH_QUEUE_RES",
    S2C_MATCH_FOUND: "MATCH_FOUND",
    S2C_LOBBY_STATE: "LOBBY_STATE",
    S2C_GAME_START: "GAME_START",
    S2D_DEDI_ASSIGN: "DEDI_ASSIGN_MATCH",
}

# ============================================================
#  Packet helpers
# ============================================================

def build_packet(opcode, body_dict):
    body = json.dumps(body_dict).encode('utf-8')
    total_len = HEADER_SIZE + len(body)
    header = struct.pack('<IH', total_len, opcode)
    return header + body

def recv_packet(sock):
    # Read header
    header_data = b''
    while len(header_data) < HEADER_SIZE:
        chunk = sock.recv(HEADER_SIZE - len(header_data))
        if not chunk:
            return None, None
        header_data += chunk

    total_len, opcode = struct.unpack('<IH', header_data)
    body_len = total_len - HEADER_SIZE

    body_data = b''
    while len(body_data) < body_len:
        chunk = sock.recv(body_len - len(body_data))
        if not chunk:
            return None, None
        body_data += chunk

    try:
        body = json.loads(body_data.decode('utf-8'))
    except:
        body = body_data.decode('utf-8', errors='replace')

    return opcode, body

# ============================================================
#  Recv thread
# ============================================================

class RecvThread(threading.Thread):
    def __init__(self, sock, state):
        super().__init__(daemon=True)
        self.sock = sock
        self.state = state
        self.running = True

    def run(self):
        while self.running:
            try:
                opcode, body = recv_packet(self.sock)
                if opcode is None:
                    print("\n[!] Disconnected from server")
                    break

                name = OPCODE_NAMES.get(opcode, f"0x{opcode:04X}")
                print(f"\n[<<] {name}: {json.dumps(body, indent=2, ensure_ascii=False)}")

                # Update state
                if opcode == S2C_LOGIN_RES and isinstance(body, dict):
                    if body.get("result") == "ok":
                        self.state["token"] = body.get("token", "")
                        self.state["nickname"] = body.get("nickname", "")
                        print(f"[*] Logged in as {self.state['nickname']}")

                elif opcode == S2C_MATCH_FOUND and isinstance(body, dict):
                    self.state["match_id"] = body.get("match_id", "")
                    print(f"[*] Match found: {self.state['match_id']}")

                elif opcode == S2C_GAME_START and isinstance(body, dict):
                    ip = body.get("server_ip", "")
                    port = body.get("server_port", 0)
                    self.state["auth_token"] = body.get("auth_token", "")
                    print(f"[*] GAME START! Connect to {ip}:{port}")

                elif opcode == S2D_DEDI_ASSIGN and isinstance(body, dict):
                    self.state["assigned_match"] = body.get("match_id", "")
                    self.state["assigned_tokens"] = body.get("auth_tokens", [])
                    print(f"[*] DEDI ASSIGNED: match {self.state['assigned_match']}")

            except (ConnectionError, OSError):
                print("\n[!] Connection lost")
                break

# ============================================================
#  Interactive mode
# ============================================================

def interactive_mode(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    print(f"[*] Connected to {host}:{port}")

    state = {"token": "", "nickname": "", "match_id": ""}
    recv = RecvThread(sock, state)
    recv.start()

    print("""
Commands:
  login <id> <pw>     - Login
  queue               - Join matchmaking queue
  cancel              - Cancel matchmaking
  ready               - Toggle ready in lobby
  team <id>           - Change team
  dedi <ip> <port>    - Register as dedi server
  heartbeat           - Send dedi heartbeat
  quit                - Exit
""")

    while True:
        try:
            line = input("> ").strip()
        except (EOFError, KeyboardInterrupt):
            break

        if not line:
            continue

        parts = line.split()
        cmd = parts[0].lower()

        if cmd == "login" and len(parts) >= 3:
            pkt = build_packet(C2S_LOGIN_REQ, {"id": parts[1], "pw": parts[2]})
            sock.sendall(pkt)
            print(f"[>>] LOGIN_REQ: {parts[1]}")

        elif cmd == "queue":
            pkt = build_packet(C2S_MATCH_QUEUE_REQ, {"token": state["token"]})
            sock.sendall(pkt)
            print("[>>] MATCH_QUEUE_REQ")

        elif cmd == "cancel":
            pkt = build_packet(C2S_MATCH_CANCEL, {"token": state["token"]})
            sock.sendall(pkt)
            print("[>>] MATCH_CANCEL")

        elif cmd == "ready":
            pkt = build_packet(C2S_LOBBY_READY, {
                "token": state["token"],
                "match_id": state["match_id"]
            })
            sock.sendall(pkt)
            print("[>>] LOBBY_READY")

        elif cmd == "team" and len(parts) >= 2:
            pkt = build_packet(C2S_LOBBY_TEAM_CHANGE, {
                "token": state["token"],
                "match_id": state["match_id"],
                "team": int(parts[1])
            })
            sock.sendall(pkt)
            print(f"[>>] LOBBY_TEAM_CHANGE: team {parts[1]}")

        elif cmd == "dedi" and len(parts) >= 3:
            pkt = build_packet(D2S_DEDI_REGISTER, {
                "ip": parts[1],
                "port": int(parts[2]),
                "max_players": 6
            })
            sock.sendall(pkt)
            print(f"[>>] DEDI_REGISTER: {parts[1]}:{parts[2]}")

        elif cmd == "heartbeat":
            pkt = build_packet(D2S_DEDI_HEARTBEAT, {
                "server_id": "self",
                "status": "idle",
                "player_count": 0
            })
            sock.sendall(pkt)
            print("[>>] DEDI_HEARTBEAT")

        elif cmd == "quit":
            break

        else:
            print(f"Unknown command: {line}")

    sock.close()

# ============================================================
#  Auto mode (login + queue + auto-ready)
# ============================================================

def auto_mode(host, port, login_id, password):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    print(f"[{login_id}] Connected to {host}:{port}")

    state = {"token": "", "nickname": "", "match_id": ""}
    recv = RecvThread(sock, state)
    recv.start()

    # Login
    time.sleep(0.3)
    sock.sendall(build_packet(C2S_LOGIN_REQ, {"id": login_id, "pw": password}))
    print(f"[{login_id}] Sent LOGIN_REQ")

    # Wait for login response
    time.sleep(1)
    if not state["token"]:
        print(f"[{login_id}] Login failed!")
        sock.close()
        return

    # Join queue
    sock.sendall(build_packet(C2S_MATCH_QUEUE_REQ, {"token": state["token"]}))
    print(f"[{login_id}] Sent MATCH_QUEUE_REQ")

    # Wait for match found
    for _ in range(60):
        time.sleep(1)
        if state["match_id"]:
            break

    if not state["match_id"]:
        print(f"[{login_id}] No match found within 60s")
        sock.close()
        return

    # Auto ready
    time.sleep(1)
    sock.sendall(build_packet(C2S_LOBBY_READY, {
        "token": state["token"],
        "match_id": state["match_id"]
    }))
    print(f"[{login_id}] Sent LOBBY_READY")

    # Wait for GAME_START
    for _ in range(30):
        time.sleep(1)
        if state.get("auth_token"):
            print(f"[{login_id}] Game started! Waiting...")
            break

    # Keep alive
    try:
        while recv.running:
            time.sleep(1)
    except KeyboardInterrupt:
        pass

    sock.close()

# ============================================================
#  Dedi simulator mode
# ============================================================

def dedi_mode(host, port, dedi_ip, dedi_port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    print(f"[DEDI] Connected to {host}:{port}")

    state = {}
    recv = RecvThread(sock, state)
    recv.start()

    # Register
    time.sleep(0.3)
    sock.sendall(build_packet(D2S_DEDI_REGISTER, {
        "ip": dedi_ip,
        "port": int(dedi_port),
        "max_players": 6
    }))
    print(f"[DEDI] Registered: {dedi_ip}:{dedi_port}")

    # Heartbeat loop
    try:
        while recv.running:
            time.sleep(10)
            sock.sendall(build_packet(D2S_DEDI_HEARTBEAT, {
                "server_id": "self",
                "status": "idle" if not state.get("assigned_match") else "ingame",
                "player_count": 0
            }))
    except KeyboardInterrupt:
        pass

    sock.close()

# ============================================================
#  Main
# ============================================================

if __name__ == "__main__":
    HOST = "127.0.0.1"
    PORT = 9000

    if len(sys.argv) >= 4 and sys.argv[1] == "--auto":
        auto_mode(HOST, PORT, sys.argv[2], sys.argv[3])
    elif len(sys.argv) >= 4 and sys.argv[1] == "--dedi":
        dedi_mode(HOST, PORT, sys.argv[2], sys.argv[3])
    else:
        interactive_mode(HOST, PORT)
