#ifndef MQTT_H
#define MQTT_H

#include <stdio.h>
#include <mosquitto.h>

#include "config_parser.h"

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code);

/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
void on_publish(struct mosquitto *mosq, void *obj, int mid);

/* This function pretends to read some data from a sensor and publish it.*/
void publish_data(mqtt_config *mqtt_conf, struct mosquitto *mosq);

int init_mosquitto(mqtt_config *mqtt_conf, struct mosquitto *mosq);

void shutdown_mosquitto();

#endif // MQTT_H