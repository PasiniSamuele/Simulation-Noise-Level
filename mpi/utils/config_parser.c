#include "config_parser.h"

int read_simulation_config(simulation_config *sim_conf) {
    config_t cfg;
    config_setting_t *sim_setting;

    config_init(&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file(&cfg, ALL_CFG_PATH)) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    /* Read a list of parameters of the simulation. */
    sim_setting = config_lookup(&cfg, "simulation");
    if (sim_setting != NULL) {

        /* Only read the record if all of the expected fields are present. */
        if ( !( config_setting_lookup_int(sim_setting, "P", &sim_conf->P)
                && config_setting_lookup_int(sim_setting, "V", &sim_conf->V)
                && config_setting_lookup_int(sim_setting, "MAX_Y", &sim_conf->MAX_Y)
                && config_setting_lookup_int(sim_setting, "MAX_X", &sim_conf->MAX_X)
                && config_setting_lookup_int(sim_setting, "Np", &sim_conf->Np)
                && config_setting_lookup_int(sim_setting, "Nv", &sim_conf->Nv)
                && config_setting_lookup_int(sim_setting, "Dp", &sim_conf->Dp)
                && config_setting_lookup_int(sim_setting, "Dv", &sim_conf->Dv)
                && config_setting_lookup_int(sim_setting, "Vp", &sim_conf->Vp)
                && config_setting_lookup_int(sim_setting, "Vv", &sim_conf->Vv)
                && config_setting_lookup_int(sim_setting, "t", &sim_conf->t))) {
            fprintf(stderr, "Could not read some simulation parameters!\n");
        }
    }
    config_destroy(&cfg);
    return EXIT_SUCCESS;
}


int read_mqtt_config(mqtt_config *mqtt_conf) {
    config_t cfg;
    config_setting_t *mqtt_setting;

    config_init(&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file(&cfg, ALL_CFG_PATH)) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    /* Read a list of parameters of mqtt. */
    mqtt_setting = config_lookup(&cfg, "mqtt");
    if (mqtt_setting != NULL) {

        /* Only read the record if all of the expected fields are present. */
        if ( !( config_setting_lookup_string(mqtt_setting, "ip", &mqtt_conf->ip)
                && config_setting_lookup_int(mqtt_setting, "port", &mqtt_conf->port)
                && config_setting_lookup_int(mqtt_setting, "keep_alive", &mqtt_conf->keep_alive)
                && config_setting_lookup_string(mqtt_setting, "username", &mqtt_conf->username)
                && config_setting_lookup_string(mqtt_setting, "password", &mqtt_conf->password)
                && config_setting_lookup_string(mqtt_setting, "topic", &mqtt_conf->topic))) {
            fprintf(stderr, "Could not read some mqtt parameters!\n");
        }
    }
    
    config_destroy(&cfg);
    return EXIT_SUCCESS;
}
