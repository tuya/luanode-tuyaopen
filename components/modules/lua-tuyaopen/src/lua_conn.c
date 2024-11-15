#include "lua.h"
#include "lauxlib.h"
#include "tal_api.h"
#include "tuya_iot_config.h"
#include "netmgr.h"
#if ENABLE_WIFI
#include "tal_wifi.h"
#endif

#define MAX_IPADDR_LEN 15

static int l_conn_ip(lua_State *L)
{
    NW_IP_S param = {0};
    if (OPRT_OK != netmgr_conn_get(NETCONN_AUTO, NETCONN_CMD_IP, &param)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, 1);
    lua_pushstring(L, param.ip);
    lua_pushstring(L, param.mask);
    lua_pushstring(L, param.mask);
    return 4;
}

static int l_conn_mac(lua_State *L)
{
    NW_MAC_S param = {0};
    if (OPRT_OK != netmgr_conn_get(NETCONN_AUTO, NETCONN_CMD_MAC, &param)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, 1);
    char buff[32] = {0};
    snprintf(buff, sizeof(buff), "%02x:%02x:%02x:%02x:%02x:%02x", 
                            param.mac[0], param.mac[1], param.mac[2], 
                            param.mac[3], param.mac[4], param.mac[5]); 
    lua_pushfstring(L, buff);
    return 2;
}

static int l_conn_state(lua_State *L)
{
    int num_args = lua_gettop(L);
    netmgr_type_e type = NETCONN_AUTO;
    if (1 <= num_args) {
        char *type = luaL_checkstring(L, 1);
        if (0 == strcmp(type, "wifi")) {
            type = NETCONN_WIFI;
        } else if (0 == strcmp(type, "wired")) {
            type = NETCONN_WIRED;
        } else {
            lua_pushstring(L, "type not supported!");
            return 1;
        }
    }

    netmgr_status_e param = NETMGR_LINK_DOWN;
    if (OPRT_OK != netmgr_conn_get(type, NETCONN_CMD_STATUS, &param)) {
        lua_pushboolean(L, NETMGR_LINK_DOWN);
        return 1;
    }

    lua_pushboolean(L, param);
    return 1;
}

static int l_conn_type(lua_State *L)
{
    int cnt = 0;
#if ENABLE_WIFI
    cnt ++;
    lua_pushstring(L, "wifi");
#endif
#if ENABLE_BLUETOOTH
    cnt ++;
    lua_pushstring(L, "ble");
#endif
#if ENABLE_WIRED
    cnt ++;
    lua_pushstring(L, "wired");
#endif

    return cnt;
}

static int l_conn_init(lua_State *L)
{
    // 初始化LWIP
#if defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
    TUYA_LwIP_Init();
#endif

    netmgr_type_e type = 0;
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
    type |= NETCONN_WIFI;
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
    type |= NETCONN_WIRED;
#endif
    netmgr_init(type);

    lua_pushboolean(L, 1);
    return 1;
}

#if ENABLE_WIFI
#include "tal_wifi.h"
static int l_conn_connect(lua_State *L)
{
    int s_len = 0, p_len = 0;
    char *ssid = luaL_checklstring(L, 1, &s_len);
    char *passwd = luaL_checklstring(L, 1, &p_len);

    tal_wifi_set_work_mode(WWM_STATION);
    if (OPRT_OK != tal_wifi_station_connect(ssid, passwd)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, 1);
    return 1;
}

static int l_conn_disconnect(lua_State *L)
{
    if (OPRT_OK != tal_wifi_station_disconnect()) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, 1);
    return 1;    
}

static int l_conn_scan(lua_State *L)
{
    int num = 10;
    AP_IF_S *ap_info = NULL;   
    if (OPRT_OK != tal_wifi_all_ap_scan(ap_info, &num)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushinteger(L, num);
    lua_newtable(L);
    int i = 0;
    for (i = 0; i < num; i++) {
        lua_newtable(L);
        lua_pushstring(L, "ssid:");
        lua_pushstring(L, ap_info[i].ssid);
        lua_settable(L, -3);


        lua_pushstring(L, "rssid:");
        lua_pushinteger(L, ap_info[i].rssi);
        lua_settable(L, -3);

        lua_rawseti(L, -2, i);
    }

    tal_wifi_release_ap(ap_info);
    return 2;    
}

static int l_conn_start_ap(lua_State *L)
{
    WF_AP_CFG_IF_S cfg = {0};
    cfg.chan = 6;
    cfg.max_conn = 3;
    cfg.ms_interval = 100;
    char *ssid = NULL;
    char *passwd = NULL;
    char *ip = NULL;
    luaL_checktype(L, 1, LUA_TTABLE);

    // ssid
    lua_getfield(L, 1, "ssid");
    if (!lua_isnil(L, -1)) {
        ssid = luaL_checklstring(L, -1, &cfg.s_len);
        if (cfg.s_len > WIFI_SSID_LEN) {            
            lua_pushboolean(L, 0);
            lua_pushstring(L, "need ssid!");
            return 2;
        } else {
            strcpy(cfg.ssid, ssid);
        }
    } else {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "ssid too long!");
        return 2;
    }  

    // pswd
    lua_getfield(L, 1, "passwd");
    if (!lua_isnil(L, -1)) {
        passwd = luaL_checklstring(L, -1, &cfg.p_len);
        if (cfg.p_len > WIFI_PASSWD_LEN) {            
            lua_pushboolean(L, 0);
            lua_pushstring(L, "passwd too long!");
            return 2;
        } else {
            strcpy(cfg.passwd, passwd);
            cfg.md = WAAM_WPA_WPA2_PSK;
        }
    } else {
        cfg.md = WAAM_OPEN;
    }

    lua_getfield(L, 1, "ip");
    if (!lua_isnil(L, -1)) {
        int len = 0;
        ip = luaL_checklstring(L, -1, &len);
        if (cfg.p_len > MAX_IPADDR_LEN) {            
            lua_pushboolean(L, 0);
            lua_pushstring(L, "ip too long!");
            return 2;
        } else {
            strcpy(cfg.ip.ip, ip);
            strcpy(cfg.ip.gw, ip);
            strcpy(cfg.ip.mask, "255.255.255.0");
        }
    } else {
        strcpy(cfg.ip.ip, "192.168.176.1");
        strcpy(cfg.ip.gw, "192.168.176.1");        
        strcpy(cfg.ip.mask, "255.255.255.0");
    }

    tal_wifi_set_work_mode(WWM_SOFTAP);
    if (OPRT_OK == tal_wifi_ap_start(&cfg)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, 1);
    return 1;

}

static int l_conn_stop_ap(lua_State *L) 
{
    tal_wifi_ap_stop();
}
#endif

#include "rotable2.h"
static const rotable_Reg_t conn_funcs[] = {
    {"init",          ROREG_FUNC(l_conn_init)},
    {"supported",     ROREG_FUNC(l_conn_type)},
    {"ip",            ROREG_FUNC(l_conn_ip)},
    {"mac",           ROREG_FUNC(l_conn_mac)},
    {"isup",          ROREG_FUNC(l_conn_state)},
#if ENABLE_WIFI
    {"connect",       ROREG_FUNC(l_conn_connect)},
    {"disconnect",    ROREG_FUNC(l_conn_disconnect)},
    {"scan",          ROREG_FUNC(l_conn_scan)},
    {"start_ap",      ROREG_FUNC(l_conn_start_ap)},
    {"stop_ap",       ROREG_FUNC(l_conn_stop_ap)},
#endif    
    {NULL, ROREG_INT(0)}
};

LUAMOD_API int luaopen_connlib(lua_State *L)
{
    luaL_newlib2(L, conn_funcs);
    return 1;
}