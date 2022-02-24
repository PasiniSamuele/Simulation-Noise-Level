#ifndef MQTT_UTIL_H_   /* Include guard */
#define MQTT_UTIL_H_

/*---------------------------------------------------------------------------*/
/** 
 *
 * Demonstrates MQTT functionality using a local Mosquitto borker. 
 * Published messages include a fake temperature reading.
 * @{
 *
 * \file
 * An MQTT example 
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "mqtt.h"
#include "rpl.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"

#include "sys/log.h"
#define LOG_MODULE "MQTT-UTIL"
#define LOG_LEVEL LOG_LEVEL_INFO

#include <string.h>

void init_config(void);
void update_config(struct etimer *mqtt_timer);
void mqtt_state_machine(struct etimer *mqtt_timer);


#endif // MQTT_UTIL_H_