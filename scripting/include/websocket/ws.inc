enum WebsocketState
{
  CONNECTING,
  OPEN,
  CLOSING,
  CLOSED
};

enum WebsocketType
{
  WebSocket_JSON,
  Websocket_STRING
}

enum AddressFamily {
  AF_INET,
  AF_INET6
}

// Define a typeset for WebSocket callbacks
typeset WebsocketCallback
{
  // OnOpen - Called when the WebSocket connection is opened
  function void (WebSocket ws);
  
  // OnMessage - Called when a text message is received
  function void (WebSocket ws, const char[] message, int wireSize);
  
  // OnMessage (JSON) - Called when a JSON message is received
  function void (WebSocket ws, const YYJSON data, int wireSize);
  
  // OnClose - Called when the WebSocket connection is closed
  function void (WebSocket ws, int code, const char[] reason);
  
  // OnError - Called when an error occurs with the WebSocket connection
  function void (WebSocket ws, const char[] errMsg);
}

// Define a methodmap for WebSocket, which extends Handle
methodmap WebSocket < Handle
{
  /**
  * Create New WebSocket Client
  * 
  * @note All callbacks need to be set before client connect
  *
  * @param url               WebSocket server URL
  * @param type              JSON or string
  */
  public native WebSocket(const char[] url, WebsocketType type);

  /**
  * Set the callback for when a message is received
  *
  * @param fOnMessage        Function to call when a message is received
  */
  public native void SetMessageCallback(WebsocketCallback fOnMessage);

  /**
  * Set the callback for when the connection is opened
  *
  * @param fOnOpen           Function to call when the connection is opened
  */
  public native void SetOpenCallback(WebsocketCallback fOnOpen);

  /**
  * Set the callback for when the connection is closed
  *
  * @param fOnClose          Function to call when the connection is closed
  */
  public native void SetCloseCallback(WebsocketCallback fOnClose);

  /**
  * Set the callback for when an error occurs
  *
  * @param fOnError          Function to call when an error occurs
  */
  public native void SetErrorCallback(WebsocketCallback fOnError);

  /**
  * Set a header for the WebSocket connection
  *
  * @param key               Header key
  * @param value             Header value
  */
  public native void SetHeader(const char[] key, const char[] value);

  /**
  * Get a header value by key
  *
  * @param key               Header key
  * @param buffer            Buffer to store the header value
  * @param maxlength         Maximum length of the buffer
  * @return                  True if the header was found, false otherwise
  */
  public native bool GetHeader(const char[] key, char[] buffer, int maxlength);

  /**
  * Send a string message over the WebSocket connection
  *
  * @param message           Message to send
  */
  public native void WriteString(const char[] message);

  /**
  * Send a JSON message over the WebSocket connection
  *
  * @param data              JSON data to send
  */
  public native void WriteJSON(const YYJSON data);

  /**
  * Open the WebSocket connection
  */
  public native void Connect();

  /**
  * Close the WebSocket connection
  */
  public native void Disconnect();

  /**
  * Get the ready state of the WebSocket connection
  *
  * @return                  Ready state
  */
  property WebsocketState ReadyState {
    public native get();
  }

  /**
  * Check if the WebSocket is connected
  *
  * @return                  True if connected, false otherwise
  */
  property bool Connected {
    public native get();
  }

  /**
  * Get or set the automatic reconnection feature
  *
  * @return                  True if automatic reconnection is enabled, false otherwise
  */
  property bool AutoReconnect {
    public native get();
    public native set(bool enable);
  }

  /**
  * Get or set the minimum wait time between reconnection attempts
  *
  * @param minWait           minimum reconnect wait time in milliseconds
  * @return                  minimum reconnect wait time in milliseconds
  */
  property int MinReconnectWait {
    public native get();
    public native set(int minWait);
  }

  /**
  * Get or set the maximum wait time between reconnection attempts
  *
  * @param maxWait           maximum reconnect wait time in milliseconds
  * @return                  maximum reconnect wait time in milliseconds
  */
  property int MaxReconnectWait {
    public native get();
    public native set(int maxWait);
  }

  /**
  * Set the handshake timeout for the WebSocket connection
  *
  * @param                   timeout timeout in seconds
  */
  property int HandshakeTimeout {
    public native set(int timeout);
  }

  /**
  * Set the ping interval for the WebSocket connection
  *
  * @param interval          interval in seconds
  * @return                  ping interval
  */
  property int PingInterval {
    public native get();
    public native set(int interval);
  }
}

/**
* Typeset for WebsocketServerCallback
* 
* A typeset defining various callback functions for WebSocket server events.
*/
typeset WebsocketServerCallback
{
  /**
  * Function to call when a connection is opened
  *
  * @param server            websocket server handle
  * @param RemoteAddr        remote address of the client
  * @param RemoteId          remote identifier of the client
  */
  function void (WebSocketServer server, const char[] RemoteAddr, const char[] RemoteId);

  /**
  * Function to call when a message is received
  *
  * @param server            websocket server handle
  * @param client            websocket client handle
  * @param message           message received
  * @param wireSize          size of the message on the wire
  * @param RemoteAddr        remote address of the client
  * @param RemoteId          remote identifier of the client
  */
  function void (WebSocketServer server, WebSocket client, const char[] message, int wireSize, const char[] RemoteAddr, const char[] RemoteId);

  /**
  * Function to call when a connection is closed
  *
  * @param server            websocket server handle
  * @param code              close code
  * @param reason            reason for closure
  * @param RemoteAddr        remote address of the client
  * @param RemoteId          remote identifier of the client
  */
  function void (WebSocketServer server, int code, const char[] reason, const char[] RemoteAddr, const char[] RemoteId);

  /**
  * Function to call when an error occurs
  *
  * @param server            websocket server handle
  * @param errMsg            error message
  * @param RemoteAddr        remote address of the client
  * @param RemoteId          remote identifier of the client
  */
  function void (WebSocketServer server, const char[] errMsg, const char[] RemoteAddr, const char[] RemoteId);
}

/**
* Methodmap for WebSocketServer
*
* A methodmap defining various native methods for a WebSocket server
*/
methodmap WebSocketServer < Handle
{
  /**
  * Create New WebSocket server
  * 
  * @note All callbacks need to be set before server startup
  *
  * @param host              host address for the server
  * @param port              port number for the server
  * @param addressfamily     address family to use. Defaults to AF_INET (IPv4)
  * @param pingInterval      interval seconds at which the server sends ping messages to clients
  */
  public native WebSocketServer(const char[] host, int port, AddressFamily addressfamily = AF_INET, int pingInterval = 60);

  /**
  * Set the callback for when a message is received
  *
  * @param fOnMessage        Function to call when a message is received
  */
  public native void SetMessageCallback(WebsocketServerCallback fOnMessage);

  /**
  * Set the callback for when a connection is opened
  *
  * @param fOnOpen           Function to call when a connection is opened
  */
  public native void SetOpenCallback(WebsocketServerCallback fOnOpen);

  /**
  * Set the callback for when a connection is closed
  *
  * @param fOnClose          Function to call when a connection is closed
  */
  public native void SetCloseCallback(WebsocketServerCallback fOnClose);

  /**
  * Set the callback for when an error occurs
  *
  * @param fOnError          Function to call when an error occurs
  */
  public native void SetErrorCallback(WebsocketServerCallback fOnError);

  /**
  * Broadcast a message to all connected clients
  *
  * @param message           message to broadcast
  */
  public native void BroadcastMessage(const char[] message);

  /**
  * Send a message to the client
  *
  * @param clientId          client id
  * @param message           message to send
  */
  public native bool SendMessageToClient(const char[] clientId, const char[] message);

  /**
  * Start the WebSocket server
  */
  public native void Start();

  /**
  * Stop the WebSocket server
  */
  public native void Stop();

  /**
  * Disable Per message deflate
  *
  * @note Set up before server startup
  */
  public native void DisableDeflate();

  /**
  * Retrieves Per message deflate is enable
  */
  public native void IsDeflateEnabled();

  /**
  * Retrieves connected clients
  */
  property int ClientsCount {
    public native get();
  }

  /**
  * Retrieve/Set Pong is enabled
  */
  property bool EnablePong {
    public native get();
    public native set(bool enablePong);
  }
}