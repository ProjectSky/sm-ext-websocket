// Define a typeset for HTTP response callbacks
typeset ResponseCallback
{
  /**
  * Function called when an HTTP response is received
  *
  * @param http         HTTP request object
  * @param body         Response body
  * @param statusCode   HTTP status code
  */
  function void (HttpRequest http, const char[] body, int statusCode);
}

// Define a methodmap for HttpRequest, which extends Handle
methodmap HttpRequest < Handle
{
  /**
  * Create a new HTTP request
  *
  * @param url      Request URL
  * @param method   HTTP method (default is "GET")
  */
  public native HttpRequest(const char[] url, const char[] method = "GET");

  /**
  * Set the callback for when an HTTP response is received
  *
  * @param fResponse    Function to call when an HTTP response is received
  */
  public native void SetResponseCallback(ResponseCallback fResponse);

 /**
 * Perform the HTTP request
 */
  public native void Perform();
}