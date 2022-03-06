#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <libconfig.h>
#include <stdlib.h>
#include <string.h>

#define ALL_CFG_PATH "conf/all.cfg"
#define SIZE_CONF_STR 30

typedef struct simulation_config_t {
    int P;
    int V;
    int MAX_Y;
    int MAX_X;
    int Np;
    int Nv;
    int Dp;
    int Dv;
    int Vp;
    int Vv;
    int t;
} simulation_config;

typedef struct mqtt_config_t {
    char ip[SIZE_CONF_STR];
    int port;
    int keep_alive;
    char username[SIZE_CONF_STR];
    char password[SIZE_CONF_STR];
    char topic[SIZE_CONF_STR];
} mqtt_config;

int read_simulation_config(simulation_config *sim_conf);
int read_mqtt_config(mqtt_config *mqtt_conf);

#endif // CONFIG_PARSER_H