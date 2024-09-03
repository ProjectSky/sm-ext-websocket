#if defined _websocket_included
  #endinput
#endif
#define _websocket_included

#include <websocket/yyjson>
#include <websocket/http>
#include <websocket/ws>

public Extension __ext_websocket = {
  name = "websocket",
  file = "websocket.ext",
#if defined AUTOLOAD_EXTENSIONS
  autoload = 1,
#else
  autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
  required = 1,
#else
  required = 0,
#endif
};
