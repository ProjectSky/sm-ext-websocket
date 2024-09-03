#include "extension.h"

enum
{
	WebSocket_JSON,
	Websocket_STRING,
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
	ix::WebSocketHttpHeaders m_extraHeaders;
	bool m_keepConnecting = false;

	IChangeableForward *pMessageForward = NULL;
	IChangeableForward *pOpenForward = NULL;
	IChangeableForward *pCloseForward = NULL;
	IChangeableForward *pErrorForward = NULL;
};