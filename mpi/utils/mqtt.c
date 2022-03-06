#include "mqtt.h"

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		mosquitto_disconnect(mosq);
	}

	/* You may wish to set a flag here to indicate to your application that the
	 * client is now connected. */
}


/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
void on_publish(struct mosquitto *mosq, void *obj, int mid) {
	printf("Message with mid %d has been published.\n", mid);
}

int publish_data(mqtt_config *mqtt_conf, struct mosquitto *mosq, noise_data *noises, int noises_size) {
	int rc, len;
	char pub_buffer[PUBLISH_BUFFER_SIZE];

	for (int i = 0; i < noises_size; i++) {
		// Skip the send if the noise_level is 0
		if (noises[i].noise_level == 0) {
			continue;
		}

		len = snprintf(pub_buffer, PUBLISH_BUFFER_SIZE, 
							"{\"noise\": %d, \"mode\": \"%s\", \"coordX\": %d, \"coordY\": %d}", 
							noises[i].noise_level, "avg", noises[i].x,  noises[i].y);

		if (len < 0 || len >= PUBLISH_BUFFER_SIZE) {
			fprintf(stderr, "Error: Buffer too short or too long. Have %d, need %d + \\0\n", PUBLISH_BUFFER_SIZE, len);
			return 1;
		}

		/* Publish the message
		* mosq - our client instance
		* *mid = NULL - we don't want to know what the message id for this message is
		* topic = "noise/region" - the topic on which this message will be published
		* payloadlen = strlen(payload) - the length of our payload in bytes
		* payload - the actual payload
		* qos = 1 - publish with QoS 1
		* retain = false - do not use the retained message feature for this message
		*/
		rc = mosquitto_publish(mosq, NULL, mqtt_conf->topic, len, pub_buffer, 1, false);
		if (rc != MOSQ_ERR_SUCCESS) {
			fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
		}
	}
}

int init_mosquitto(mqtt_config *mqtt_conf, struct mosquitto **ptr_mosq) {
	int rc;

	/* Required before calling other mosquitto functions */
	mosquitto_lib_init();

	/* Create a new client instance.
	 * id = NULL -> ask the broker to generate a client id for us
	 * clean session = true -> the broker should remove old sessions when we connect
	 * obj = NULL -> we aren't passing any of our private data for callbacks
	 */
	*ptr_mosq = mosquitto_new(NULL, true, NULL);
	if(*ptr_mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	
    rc = mosquitto_username_pw_set(*ptr_mosq, mqtt_conf->username, mqtt_conf->password);
    if(rc != MOSQ_ERR_SUCCESS){
        fprintf(stderr, "Error: Failed to login.\n");
        return 1;
    }
	
	/* Configure callbacks. This should be done before connecting ideally. */
	mosquitto_connect_callback_set(*ptr_mosq, on_connect);
	mosquitto_publish_callback_set(*ptr_mosq, on_publish);

	/* Connect to mosquitto server on port 1883, with a keepalive of 60 seconds.
	 * This call makes the socket connection only, it does not complete the MQTT
	 * CONNECT/CONNACK flow, you should use mosquitto_loop_start() or
	 * mosquitto_loop_forever() for processing net traffic. */
	rc = mosquitto_connect(*ptr_mosq, mqtt_conf->ip, mqtt_conf->port, mqtt_conf->keep_alive);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(*ptr_mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	/* Run the network loop in a background thread, this call returns quickly. */
	rc = mosquitto_loop_start(*ptr_mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(*ptr_mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	/* At this point the client is connected to the network socket, but may not
	 * have completed CONNECT/CONNACK.
	 * It is fairly safe to start queuing messages at this point, but if you
	 * want to be really sure you should wait until after a successful call to
	 * the connect callback.
	 * In this case we know it is 1 second before we start publishing.
	 */
}

void shutdown_mosquitto() {
    mosquitto_lib_cleanup();
}
