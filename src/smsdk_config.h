#ifndef _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_

#define SMEXT_CONF_NAME			"SourceMod WebSocket Extension"
#define SMEXT_CONF_DESCRIPTION	"Provide JSON and WebSocket Native"
#define SMEXT_CONF_VERSION		"1.0.1"
#define SMEXT_CONF_AUTHOR		"ProjectSky"
#define SMEXT_CONF_URL			"https://github.com/ProjectSky/sm-ext-websocket"
#define SMEXT_CONF_LOGTAG		"websocket"
#define SMEXT_CONF_LICENSE		"GPL"
#define SMEXT_CONF_DATESTRING	__DATE__

#define SMEXT_LINK(name) SDKExtension *g_pExtensionIface = name;

#define SMEXT_ENABLE_HANDLESYS
#define SMEXT_ENABLE_FORWARDSYS

#endif // _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
