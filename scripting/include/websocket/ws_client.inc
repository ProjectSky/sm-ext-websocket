enum WebSocketState
{
  CONNECTING,
  OPEN,
  CLOSING,
  CLOSED
}

enum WebsocketType
{
  WebSocket_JSON,
  WebSocket_STRING,
  WebSocket_BINARY
}

// Define a typeset for WebSocket callbacks
typeset WebsocketCallback
{
  /**
  * OnOpen - Called when the WebSocket connection is opened
  * @param ws                WebSocket handle
  */
  function void (WebSocket ws);

  /**
  * OnMessage (STRING) - Called when a text message is received
  * @param ws                WebSocket handle
  * @param message           message received
  * @param wireSize          size of the message on the wire (including null terminator)
  */
  function void (WebSocket ws, const char[] message, int wireSize);

  /**
  * OnMessage (JSON) - Called when a JSON message is received
  * @param ws                WebSocket handle
  * @param data              JSON data
  * @param wireSize          size of the message on the wire (including null terminator)
  */
  function void (WebSocket ws, const JSON data, int wireSize);

  /**
  * OnMessage (BINARY) - Called when a BINARY message is received
  * @param ws                WebSocket handle
  * @param data              binary data
  * @param wireSize          size of the message on the wire (including null terminator)
  */
  function void (WebSocket ws, const char[] data, int wireSize);

  /**
  * OnClose - Called when the WebSocket connection is closed
  * @param ws                WebSocket handle
  * @param code              close code
  * @param reason            reason for closure
  */
  function void (WebSocket ws, int code, const char[] reason);

  /**
  * OnError - Called when an error occurs with the WebSocket connection
  * @param ws                WebSocket handle
  * @param errMsg            error message
  */
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
  * @param type              JSON or string. Defaults to string
  */
  public native WebSocket(const char[] url, WebsocketType type = WebSocket_STRING);

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
  * Send a binary message over the WebSocket connection
  *
  * @param data              Byte array to send 
  * @param length            Length of the binary data
  *
  * @return                  Success status 
  */
  public native bool WriteBinary(const char[] data, int length);

  /**
  * Send a string message over the WebSocket connection
  *
  * @param message           Message to send
  *
  * @return                  Success status 
  */
  public native bool WriteString(const char[] message);

  /**
  * Send a JSON message over the WebSocket connection
  *
  * @param data              JSON data to send
  * @param write_flg         Write flags (JSON_WRITE_FLAG values, default: JSON_WRITE_NOFLAG)
  *
  * @return                  Success status 
  */
  public native bool WriteJSON(const JSON data, JSON_WRITE_FLAG write_flg = JSON_WRITE_NOFLAG);

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
  property WebSocketState ReadyState {
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
  * Retrieve/Set the automatic reconnection feature
  *
  * @return                  True if automatic reconnection is enabled, false otherwise
  */
  property bool AutoReconnect {
    public native get();
    public native set(bool enable);
  }

  /**
  * Retrieve/Set the minimum wait time between reconnection attempts
  *
  * @return                  minimum reconnect wait time in milliseconds
  */
  property int MinReconnectWait {
    public native get();
    public native set(int minWait);
  }

  /**
  * Retrieve/Set the maximum wait time between reconnection attempts
  *
  * @return                  maximum reconnect wait time in milliseconds
  */
  property int MaxReconnectWait {
    public native get();
    public native set(int maxWait);
  }

  /**
  * Retrieve/Set the handshake timeout in seconds
  *
  * Maximum time allowed for the WebSocket handshake to complete.
  * If the handshake takes longer than this, the connection fails.
  *
  * @note                    Default: 5 seconds
  * @note                    Must be called before Connect()
  */
  property int HandshakeTimeout {
    public native get();
    public native set(int timeout);
  }

  /**
  * Retrieve/Set the ping interval in seconds
  *
  * @note                    Default: -1 (disabled)
  * @note                    Must be called before Connect()
  */
  property int PingInterval {
    public native get();
    public native set(int interval);
  }

  /**
  * Set TLS certificate and key files
  *
  * @param certFile          Path to certificate file
  * @param keyFile           Path to key file
  * @note                    Must be called before Connect()
  */
  public native void SetTLSCertAndKey(const char[] certFile, const char[] keyFile);

  /**
  * Set TLS CA file for server certificate verification
  *
  * @param caFile            Path to CA file for verifying server certificates
  *                          - "SYSTEM": Use system CA bundle to verify server (default, recommended)
  *                          - "NONE": Disable server certificate verification (UNSAFE, testing only!)
  *                          - Path: Custom CA file to trust specific server certificates
  * @note                    Must be called before Connect()
  * @note                    This verifies the SERVER's identity, not the client's
  */
  public native void SetTLSCAFile(const char[] caFile);

  /**
  * Set TLS ciphers
  *
  * @param ciphers           Cipher list (e.g., "DEFAULT", "rsa...")
  * @note                    Must be called before Connect()
  */
  public native void SetTLSCiphers(const char[] ciphers);

  /**
  * Retrieve/Set whether TLS is enabled
  *
  * @note                    Must be called before Connect()
  */
  property bool TLSEnabled {
    public native get();
    public native set(bool enable);
  }

  /**
  * Retrieve/Set hostname validation for server certificates
  *
  * @note                    Must be called before Connect()
  * @note                    When true, verifies server certificate hostname matches the connection URL
  * @note                    Recommended: keep enabled (default) to prevent MITM attacks
  */
  property bool HostnameValidation {
    public native get();
    public native set(bool enable);
  }

  /**
  * Retrieve/Set the ping timeout in seconds
  *
  * Time to wait for a pong response after sending a ping
  * If no pong is received within this time, the connection is closed
  * Requires PingInterval > 0 to function (pings must be sent first)
  * Set to -1 to disable
  *
  * @note                    Default: -1 (disabled)
  * @note                    Must be called before Connect()
  */
  property int PingTimeout {
    public native get();
    public native set(int secs);
  }

  /**
  * Retrieve/Set the idle timeout in seconds
  *
  * Time allowed without receiving any data messages (TEXT/BINARY)
  * Ping/pong frames do not reset this timer
  * If no data is received within this time, the connection is closed
  * Set to -1 to disable
  *
  * @note                    Default: -1 (disabled)
  * @note                    Must be called before Connect()
  */
  property int IdleTimeout {
    public native get();
    public native set(int secs);
  }

  /**
  * Retrieve/Set the send timeout in seconds
  *
  * Maximum time allowed for a send operation to complete.
  * If sending takes longer than this, the operation fails
  *
  * @note                    Default: 300 seconds
  * @note                    Must be called before Connect()
  */
  property int SendTimeout {
    public native get();
    public native set(int secs);
  }

  /**
  * Retrieve/Set the close timeout in seconds
  *
  * Time to wait for a clean close handshake to complete.
  * After initiating close, if the peer doesn't respond within this time,
  * the connection is forcibly terminated
  *
  * @note                    Default: 5 seconds
  * @note                    Must be called before Connect()
  */
  property int CloseTimeout {
    public native get();
    public native set(int secs);
  }

  /**
  * Get the number of messages sent
  */
  property int MessagesSent {
    public native get();
  }

  /**
  * Get the number of messages received
  */
  property int MessagesReceived {
    public native get();
  }

  /**
  * Get the number of bytes sent
  */
  property int BytesSent {
    public native get();
  }

  /**
  * Get the number of bytes received
  */
  property int BytesReceived {
    public native get();
  }

  /**
  * Get the connection duration in seconds
  */
  property int ConnectionDuration {
    public native get();
  }

  /**
  * Get the number of pings sent
  */
  property int PingsSent {
    public native get();
  }

  /**
  * Get the number of pongs sent
  */
  property int PongsSent {
    public native get();
  }

  /**
  * Get the number of pings received
  */
  property int PingsReceived {
    public native get();
  }

  /**
  * Get the number of pongs received
  */
  property int PongsReceived {
    public native get();
  }

  /**
  * Get the buffered amount (bytes waiting to be sent)
  */
  property int BufferedAmount {
    public native get();
  }

  /**
  * Reset connection statistics
  */
  public native void ResetStats();

  /**
  * Add a WebSocket subprotocol
  *
  * @param protocol          Subprotocol name to add
  * @note                    Must be called before Connect()
  */
  public native void AddSubProtocol(const char[] protocol);

  /**
  * Remove a WebSocket subprotocol
  *
  * @param protocol          Subprotocol name to remove
  * @note                    Must be called before Connect()
  */
  public native void RemoveSubProtocol(const char[] protocol);

  /**
  * Clear all WebSocket subprotocols
  *
  * @note                    Must be called before Connect()
  */
  public native void ClearSubProtocols();

  /**
  * Set proxy for WebSocket connection
  *
  * @param proxyUrl          Proxy URL (e.g., "http://proxy:8080", "socks5://proxy:1080")
  * @note                    Supports HTTP, HTTPS, and SOCKS5 proxies
  * @note                    Must be called before Connect()
  */
  public native void SetProxy(const char[] proxyUrl);

  /**
  * Clear proxy configuration
  */
  public native void ClearProxy();

  /**
  * Check if proxy is configured
  */
  property bool HasProxy {
    public native get();
  }
}

/**
* Set the User-Agent header for WebSocket client connections
*
* @param userAgent           User-Agent string
* @note                      This is a global setting that affects all WebSocket clients
*/
native void WebSocket_SetUserAgent(const char[] userAgent);

/**
* Get the current User-Agent header
*
* @param buffer              Buffer to store the User-Agent string
* @param maxlength           Maximum length of the buffer
*/
native void WebSocket_GetUserAgent(char[] buffer, int maxlength);

/**
* Set the Server header for WebSocket server responses
*
* @param server              Server header string
* @note                      This is a global setting that affects all WebSocket servers
*/
native void WebSocket_SetServerHeader(const char[] server);

/**
* Get the current Server header
*
* @param buffer              Buffer to store the Server header string
* @param maxlength           Maximum length of the buffer
*/
native void WebSocket_GetServerHeader(char[] buffer, int maxlength);
