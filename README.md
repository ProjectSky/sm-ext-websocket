# SourceMod WebSocket Extension

## What is this?
This is a [SourceMod](http://www.sourcemod.net/) extension that provides some methods for HTTP JSON and websocket communication

## Features
* Relies on [YYJSON](https://github.com/ibireme/yyjson) which A high performance JSON library written in ANSI C
* Relies on [IXWebSocket](https://github.com/machinezone/IXWebSocket) which is C++ library for WebSocket client and server development. It has minimal dependencies
* Support TEXT and JSON data
* Support client and server
* Support permessage-deflate
* Support SSL
* Support x64

## How to build this?
``` sh
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install clang g++-multilib zlib1g-dev libssl-dev zlib1g-dev:i386 libssl-dev:i386
clone project
mkdir build && cd build
python ../configure.py --enable-optimize --symbol-files --sm-path=YOU_SOURCEMOD_PATH --targets=x86,x64
ambuild
```

## Native
* [json](https://github.com/ProjectSky/sm-ext-websocket/blob/main/scripting/include/websocket/yyjson.inc)
* [websocket](https://github.com/ProjectSky/sm-ext-websocket/blob/main/scripting/include/websocket/ws.inc)

# Binary files
* https://github.com/ProjectSky/sm-ext-websocket/actions

## TODO
- [x] WebSocket server support
- [x] Windows support
- [ ] HTTP support? (Because this is a WebSocket library I might not support it)

## NOTES
* HTTP functionality is not yet complete. Currently, only basic features are available
* Server will not process data during the hibernation. You can set sv_hibernate_when_empty to 0 to disable hibernation

## Client Example
``` c++
#include <sourcemod>
#include <websocket>

WebSocket
g_hWS;

public void OnPluginStart()
{
  RegServerCmd("ws_start", ws_start);
  RegServerCmd("ws_close", ws_close);
  RegServerCmd("ws_state", ws_state);
}

Action ws_start(int args)
{
  g_hWS = new WebSocket("ws://127.0.0.1:9999", WebSocket_JSON);
  g_hWS.SetMessageCallback(onMessage);
  g_hWS.SetOpenCallback(onOpen);
  g_hWS.SetCloseCallback(onClose);
  g_hWS.SetErrorCallback(onError)
  g_hWS.Connect();
}

Action ws_close(int args)
{
  g_hWS.Disconnect();
  return Plugin_Handled;
}

Action ws_state(int args)
{
  PrintToServer("ReadyState: %d", g_hWS.ReadyState);
  return Plugin_Handled;
}

void onOpen(WebSocket ws)
{
  PrintToServer("onOpen: %x", ws);
}

void onClose(WebSocket ws, int code, const char[] reason)
{
  PrintToServer("onClose: %d, %s", code, reason);
}

void onMessage(WebSocket ws, const YYJSON message, int wireSize)
{
  char[] buffer = new char[wireSize];
  message.ToString(buffer, wireSize);
  PrintToServer("msg: %s, size: %d", buffer, wireSize);
}

void onError(WebSocket ws, const char[] errMsg)
{
  PrintToServer("onError: %s", errMsg);
}
```

## Server Example
``` c++
#include <sourcemod>
#include <websocket>

WebSocketServer
g_hWsServer;

public void OnPluginStart()
{
  RegServerCmd("ws_server", ws_server);
  RegServerCmd("ws_server_stop", ws_server_stop);
  RegServerCmd("ws_server_broadcast", ws_server_broadcast);
  RegServerCmd("ws_server_clients", ws_server_clients);
  RegServerCmd("ws_server_sendtoclient", ws_server_sendtoclient);
}

Action ws_server(int args)
{
  g_hWsServer = new WebSocketServer("0.0.0.0", 9999);
  g_hWsServer.SetMessageCallback(onSrvMessage);
  g_hWsServer.SetOpenCallback(onSrvOpen);
  g_hWsServer.SetCloseCallback(onSrvClose);
  g_hWsServer.SetErrorCallback(onSrvError);
  g_hWsServer.Start();
  return Plugin_Handled;
}

Action ws_server_broadcast(int args)
{
  g_hWsServer.BroadcastMessage("Broadcast Message");
  return Plugin_Handled;
}

Action ws_server_sendtoclient(int args)
{
  char clientId[4], message[256];
  GetCmdArg(1, clientId, sizeof(clientId));
  GetCmdArg(2, message, sizeof(message));

  g_hWsServer.SendMessageToClient(clientId, message);
  return Plugin_Handled;
}

Action ws_server_clients(int args)
{
  PrintToServer("Connected clients: %d", g_hWsServer.ClientsCount)
  return Plugin_Handled;
}

Action ws_server_stop(int args)
{
  g_hWsServer.Stop();
  return Plugin_Handled;
}

void onSrvMessage(WebSocketServer ws, WebSocket client, const char[] message, int wireSize, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("message: %s, wireSize: %d, RemoteAddr: %s, RemoteId: %s", message, wireSize, RemoteAddr, RemoteId);
}

void onSrvError(WebSocketServer ws, const char[] errMsg, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("onError: %s, RemoteAddr: %s, RemoteId: %s", errMsg, RemoteAddr, RemoteId);
}

void onSrvOpen(WebSocketServer ws, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("onOpen: %x, RemoteAddr: %s, RemoteId: %s", ws, RemoteAddr, RemoteId);
}

void onSrvClose(WebSocketServer ws, int code, const char[] reason, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("onClose: %d, reason: %s, RemoteAddr: %s, RemoteId: %s", code, reason, RemoteAddr, RemoteId);
}
```