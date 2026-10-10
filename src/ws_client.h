#ifndef WEBSOCKETEXTENSION_WS_CLIENT_H
#define WEBSOCKETEXTENSION_WS_CLIENT_H

#include "extension.h"

enum
{
	WebSocket_JSON,
	WebSocket_STRING,
	WebSocket_BINARY,
};

class WebSocketClient
{
public:
	WebSocketClient(const char *url, uint8_t callbacktype);
	WebSocketClient(ix::WebSocket* existingSocket);
	~WebSocketClient();

	bool IsConnected();

public:
	void OnMessage(const std::string &message);
	void OnOpen(ix::WebSocketOpenInfo openInfo);
	void OnClose(ix::WebSocketCloseInfo closeInfo);
	void OnError(ix::WebSocketErrorInfo errorInfo);

	ix::WebSocket* m_webSocket;
	Handle_t m_websocket_handle = BAD_HANDLE;
	Handle_t m_json_handle = BAD_HANDLE;

	uint8_t m_callback_type;
	ix::WebSocketHttpHeaders m_headers;
	std::mutex m_headersMutex;
	ix::WebSocketHttpHeaders m_extraHeaders;
	ix::SocketTLSOptions m_tlsOptions;
	ix::ProxyConfig m_proxyConfig;
	bool m_keepConnecting = false;

	IChangeableForward *pMessageForward = nullptr;
	IChangeableForward *pOpenForward = nullptr;
	IChangeableForward *pCloseForward = nullptr;
	IChangeableForward *pErrorForward = nullptr;
};

class WsMessageTaskContext : public ITaskContext
{
public:
	WsMessageTaskContext(WebSocketClient* client, const std::string& message)
		: m_client(client), m_message(message) {}

	virtual void OnCompleted() override;
	virtual void* GetOwner() override { return m_client; }

private:
	WebSocketClient* m_client;
	std::string m_message;
};

class WsOpenTaskContext : public ITaskContext
{
public:
	WsOpenTaskContext(WebSocketClient* client, ix::WebSocketOpenInfo openInfo)
		: m_client(client), m_openInfo(openInfo) {}

	virtual void OnCompleted() override;
	virtual void* GetOwner() override { return m_client; }

private:
	WebSocketClient* m_client;
	ix::WebSocketOpenInfo m_openInfo;
};

class WsCloseTaskContext : public ITaskContext
{
public:
	WsCloseTaskContext(WebSocketClient* client, ix::WebSocketCloseInfo closeInfo)
		: m_client(client), m_closeInfo(closeInfo) {}

	virtual void OnCompleted() override;
	virtual void* GetOwner() override { return m_client; }

private:
	WebSocketClient* m_client;
	ix::WebSocketCloseInfo m_closeInfo;
};

class WsErrorTaskContext : public ITaskContext
{
public:
	WsErrorTaskContext(WebSocketClient* client, ix::WebSocketErrorInfo errorInfo)
		: m_client(client), m_errorInfo(errorInfo) {}

	virtual void OnCompleted() override;
	virtual void* GetOwner() override { return m_client; }

private:
	WebSocketClient* m_client;
	ix::WebSocketErrorInfo m_errorInfo;
};
#endif // WEBSOCKETEXTENSION_WS_CLIENT_H
