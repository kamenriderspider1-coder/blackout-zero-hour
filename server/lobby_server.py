#!/usr/bin/env python3
"""Small standard-library TCP lobby/relay server for BLACKOUT: ZERO HOUR.

Protocol: one JSON object per line.
Client sends: {"type":"join","name":"Player"}
Server replies: {"type":"welcome","player_id":1,"room":"..."}
Client sends: {"type":"state","x":2.5,"y":2.5,"angle":0.0,"signals":0}
Server broadcasts: {"type":"peer_state","player_id":1,"x":...,"y":...,"angle":...,"signals":...}
Client sends: {"type":"ping"}
Server replies: {"type":"pong"}
Client sends: {"type":"leave"} or closes connection.

This is a prototype relay, not production matchmaking. Put it behind TLS
(wss/TLS TCP proxy) and add authentication/rate limiting before public use.
"""
import asyncio
import json
import os
import secrets
import signal
from dataclasses import dataclass, field

HOST = os.environ.get("BLACKOUT_HOST", "0.0.0.0")
PORT = int(os.environ.get("BLACKOUT_PORT", "8765"))
MAX_PLAYERS_PER_ROOM = 2
MAX_LINE = 2048

@dataclass
class Room:
    code: str
    peers: dict[int, asyncio.StreamWriter] = field(default_factory=dict)
    next_id: int = 1
    lock: asyncio.Lock = field(default_factory=asyncio.Lock)

rooms: dict[str, Room] = {}

async def send(writer: asyncio.StreamWriter, payload: dict) -> None:
    writer.write((json.dumps(payload, separators=(",", ":")) + "\n").encode())
    await writer.drain()

async def broadcast(room: Room, payload: dict, exclude: int | None = None) -> None:
    stale = []
    for pid, writer in list(room.peers.items()):
        if pid == exclude:
            continue
        try:
            await send(writer, payload)
        except (ConnectionError, asyncio.CancelledError):
            stale.append(pid)
    for pid in stale:
        room.peers.pop(pid, None)

async def handle(reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> None:
    room = None
    player_id = None
    try:
        raw = await asyncio.wait_for(reader.readline(), timeout=15)
        if not raw or len(raw) > MAX_LINE:
            return
        try:
            hello = json.loads(raw)
        except (UnicodeDecodeError, json.JSONDecodeError):
            await send(writer, {"type":"error","message":"Invalid join message"})
            return
        if hello.get("type") != "join":
            await send(writer, {"type":"error","message":"Send join first"})
            return
        room_code = str(hello.get("room", "PUBLIC")).upper()[:12]
        if not room_code.isalnum():
            await send(writer, {"type":"error","message":"Room code must be letters or numbers"})
            return
        room = rooms.setdefault(room_code, Room(room_code))
        async with room.lock:
            if len(room.peers) >= MAX_PLAYERS_PER_ROOM:
                await send(writer, {"type":"error","message":"Room is full"})
                return
            player_id = room.next_id
            room.next_id += 1
            room.peers[player_id] = writer
            await send(writer, {"type":"welcome","player_id":player_id,"room":room.code,"players":len(room.peers)})
            await broadcast(room, {"type":"peer_joined","player_id":player_id,"players":len(room.peers)}, exclude=player_id)
        while True:
            raw = await asyncio.wait_for(reader.readline(), timeout=45)
            if not raw:
                break
            if len(raw) > MAX_LINE:
                await send(writer, {"type":"error","message":"Message too large"})
                break
            try:
                msg = json.loads(raw)
            except (UnicodeDecodeError, json.JSONDecodeError):
                await send(writer, {"type":"error","message":"Invalid JSON"})
                continue
            kind = msg.get("type")
            if kind == "ping":
                await send(writer, {"type":"pong"})
            elif kind == "state":
                try:
                    state = {
                        "type":"peer_state",
                        "player_id":player_id,
                        "x":max(0.0,min(16.0,float(msg["x"]))),
                        "y":max(0.0,min(16.0,float(msg["y"]))),
                        "angle":float(msg["angle"]),
                        "signals":max(0,min(3,int(msg["signals"])))
                    }
                except (KeyError, TypeError, ValueError):
                    await send(writer, {"type":"error","message":"Invalid player state"})
                    continue
                await broadcast(room, state, exclude=player_id)
            elif kind == "leave":
                break
            else:
                await send(writer, {"type":"error","message":"Unknown message type"})
    except (asyncio.TimeoutError, ConnectionError):
        pass
    finally:
        if room is not None and player_id is not None:
            async with room.lock:
                room.peers.pop(player_id, None)
                await broadcast(room, {"type":"peer_left","player_id":player_id,"players":len(room.peers)})
                if not room.peers:
                    rooms.pop(room.code, None)
        writer.close()
        try:
            await writer.wait_closed()
        except ConnectionError:
            pass

async def main() -> None:
    server = await asyncio.start_server(handle, HOST, PORT, limit=MAX_LINE + 1)
    print(f"BLACKOUT lobby relay listening on {HOST}:{PORT}")
    async with server:
        await server.serve_forever()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
