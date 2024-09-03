#include "extension.h"


class WebSocketServer
{
public:
	WebSocketServer(const std::string& host, int port, int addressFamily, int pingInterval);
	~WebSocketServer();

public:
	void OnMessage(const std::string &message, std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket* client);
	void OnOpen(ix::WebSocketOpenInfo openInfo, std::shared_ptr<ix::ConnectionState> connectionState);
	void OnClose(ix::WebSocketCloseInfo closeInfo, std::shared_ptr<ix::ConnectionState> connectionState);
	void OnError(ix::WebSocketErrorInfo errorInfo, std::shared_ptr<ix::ConnectionState> connectionState);
	void broadcastMessage(const std::string& message);
	bool sendToClient(const std::string& clientId, const std::string& message);
	
	ix::WebSocketServer m_webSocketServer;
	Handle_t m_webSocketServer_handle = BAD_HANDLE;

	IChangeableForward *pMessageForward = NULL;
	IChangeableForward *pOpenForward = NULL;
	IChangeableForward *pCloseForward = NULL;
	IChangeableForward *pErrorForward = NULL;
};