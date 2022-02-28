#include "contiki.h"
#include "mqtt.h"
#include "rpl.h"
#include "dev/cooja-radio.h"
#include "dev/position_intf.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define LOG_MODULE "MQTT-UTIL"
#define LOG_LEVEL LOG_LEVEL_INFO

#define MAX_WINDOW_SIZE 6
#define AVG_THRESHOLD_DB 70
#define PARSE_BUFFER_SIZE 15
#define NOISE_CHANNEL 5
#define RPL_CHANNEL 26

#define FILENAME "test1.csv"

static char* noise_values[MAX_WINDOW_SIZE];
static uint16_t position;

static struct etimer mqtt_timer;

static int32_t fd;
static char buf[64];
static char message[32];
static char* x;
static char* y;
static char* region;

/*---------------------------------------------------------------------------*/
/*
 * Publish to a local MQTT broker (e.g. mosquitto) running on
 * the node that hosts your border router
 */
static const char *broker_ip = MQTT_BROKER_IP_ADDR;
#define DEFAULT_ORG_ID              "mqtt-util"
/*---------------------------------------------------------------------------*/
/*
 * A timeout used when waiting for something to happen (e.g. to connect or to
 * disconnect)
 */
#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)
/*---------------------------------------------------------------------------*/
/* Connections and reconnections */
#define RETRY_FOREVER              0xFF
#define RECONNECT_INTERVAL         (CLOCK_SECOND * 2)
/*---------------------------------------------------------------------------*/
/*
 * Number of times to try reconnecting to the broker.
 * Can be a limited number (e.g. 3, 10 etc) or can be set to RETRY_FOREVER
 */
#define RECONNECT_ATTEMPTS         RETRY_FOREVER
#define CONNECTION_STABLE_TIME     (CLOCK_SECOND * 5)
static struct timer connection_life;
static uint8_t connect_attempt;
/*---------------------------------------------------------------------------*/
/* Various states */
static uint8_t state;
#define STATE_INIT            0
#define STATE_REGISTERED      1
#define STATE_CONNECTING      2
#define STATE_CONNECTED       3
#define STATE_DISCONNECTED    4
#define STATE_NEWCONFIG       5
#define STATE_SAMPLING        6
#define STATE_CONFIG_ERROR 0xFE
#define STATE_ERROR        0xFF
/*---------------------------------------------------------------------------*/
#define CONFIG_ORG_ID_LEN        32
#define CONFIG_TYPE_ID_LEN       32
#define CONFIG_AUTH_TOKEN_LEN    32
#define CONFIG_CMD_TYPE_LEN       8
#define CONFIG_IP_ADDR_STR_LEN   64
/*---------------------------------------------------------------------------*/
/* A timeout used when waiting to connect to a network */
#define NET_CONNECT_PERIODIC        (CLOCK_SECOND >> 2)
/*---------------------------------------------------------------------------*/
/* Default configuration values */
#define DEFAULT_TYPE_ID             "native"
#define DEFAULT_AUTH_TOKEN          "AUTHZ"
#define DEFAULT_SUBSCRIBE_CMD_TYPE  "+"
#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_PUBLISH_INTERVAL    (10 * CLOCK_SECOND)
#define DEFAULT_KEEP_ALIVE_INTERVAL 180
/*---------------------------------------------------------------------------*/

PROCESS(noise_simulated_sensor_process, "Noise simulated sensor process");
AUTOSTART_PROCESSES(&noise_simulated_sensor_process);

/*---------------------------------------------------------------------------*/
/**
 * \brief Data structure declaration for the MQTT client configuration
 */
typedef struct mqtt_client_config {
  char org_id[CONFIG_ORG_ID_LEN];
  char type_id[CONFIG_TYPE_ID_LEN];
  char auth_token[CONFIG_AUTH_TOKEN_LEN];
  char broker_ip[CONFIG_IP_ADDR_STR_LEN];
  char cmd_type[CONFIG_CMD_TYPE_LEN];
  clock_time_t pub_interval;
  uint16_t broker_port;
} mqtt_client_config_t;
/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
/*---------------------------------------------------------------------------*/
/*
 * Buffers for Client ID and Topic.
 * Make sure they are large enough to hold the entire respective string
 *
 * We also need space for the null termination
 */
#define BUFFER_SIZE 64
static char client_id[BUFFER_SIZE];
static char pub_topic[BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
/*
 * The main MQTT buffers.
 * We will need to increase if we start publishing more data.
 */
#define PUBLISH_BUFFER_SIZE 512
static struct mqtt_connection conn;
static char pub_buffer[PUBLISH_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
static mqtt_client_config_t conf;
/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED: {
    LOG_INFO("Application has a MQTT connection!\n");
    timer_set(&connection_life, CONNECTION_STABLE_TIME);
    state = STATE_CONNECTED;
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    LOG_INFO("MQTT Disconnect: reason %u\n", *((mqtt_event_t *)data));

    state = STATE_DISCONNECTED;
    process_poll(&noise_simulated_sensor_process);
    break;
  }
  case MQTT_EVENT_PUBACK: {
    LOG_INFO("Publishing complete\n");
    break;
  }
  default:
    LOG_WARN("Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static int
construct_pub_topic(void)
{
  int len = snprintf(pub_topic, BUFFER_SIZE, MQTT_PUBLISH_TOPIC);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    LOG_ERR("Pub topic: %d, buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_client_id(void)
{
  int len = snprintf(client_id, BUFFER_SIZE, "d:%s:%s:%02x%02x%02x%02x%02x%02x",
                     conf.org_id, conf.type_id,
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    LOG_INFO("Client ID: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
update_config()
{
  if(construct_client_id() == 0) {
    /* Fatal error. Client ID larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  if(construct_pub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  state = STATE_INIT;

  /*
   * Schedule next timer event ASAP
   *
   * If we entered an error state then we won't do anything when it fires
   *
   * Since the error at this stage is a config error, we will only exit this
   * error state if we get a new config
   */
  etimer_set(&mqtt_timer, 0);

  return;
}
/*---------------------------------------------------------------------------*/
static void
init_config(void)
{
  /* Populate configuration with default values */
  memset(&conf, 0, sizeof(mqtt_client_config_t));

  memcpy(conf.org_id, DEFAULT_ORG_ID, strlen(DEFAULT_ORG_ID));
  memcpy(conf.type_id, DEFAULT_TYPE_ID, strlen(DEFAULT_TYPE_ID));
  memcpy(conf.auth_token, DEFAULT_AUTH_TOKEN, strlen(DEFAULT_AUTH_TOKEN));
  memcpy(conf.broker_ip, broker_ip, strlen(broker_ip));
  memcpy(conf.cmd_type, DEFAULT_SUBSCRIBE_CMD_TYPE, 1);

  conf.broker_port = DEFAULT_BROKER_PORT;
  conf.pub_interval = DEFAULT_PUBLISH_INTERVAL;
}
/*---------------------------------------------------------------------------*/
static void
publish(char *value)
{
  int len = snprintf(pub_buffer, PUBLISH_BUFFER_SIZE, "{\"noise\": %s, \"X\": %s, \"Y\": %s, \"region\": %s}", value, x, y , region);

  if(len < 0 || len >= PUBLISH_BUFFER_SIZE) {
    LOG_ERR("Buffer too short. Have %d, need %d + \\0\n", PUBLISH_BUFFER_SIZE, len);
    return;
  }

  LOG_INFO("Publishing %s\n", pub_buffer);

  mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)pub_buffer,
               len, MQTT_QOS_LEVEL_1, MQTT_RETAIN_OFF);

  LOG_INFO("Publish sent out!\n");
}

static void
publish_avg(double avg) {
  char avg_string[PARSE_BUFFER_SIZE];
  snprintf(avg_string, PARSE_BUFFER_SIZE, "\"%.2f\"", avg);

  publish(avg_string);
}

static void
publish_raw(void) {
  char double_string[PARSE_BUFFER_SIZE];
  char final_string[MAX_WINDOW_SIZE * PARSE_BUFFER_SIZE] = "[";  

  for (size_t i = 0; i < MAX_WINDOW_SIZE; i++) {    
    snprintf(double_string, PARSE_BUFFER_SIZE, "\"%s\",", noise_values[i]);
    strcat(final_string, double_string);
  }

  size_t len = strlen(final_string);

  // Replaces last ',' with ']'
  final_string[len - 1] = ']';

  publish(final_string);
}

static void
publish_noise(void) {
  double avg = 0;
  
  for (size_t i = 0; i < MAX_WINDOW_SIZE; i++) {
    avg += noise_values[i] - "0";
  }
  
  avg /= MAX_WINDOW_SIZE;

  if (avg < AVG_THRESHOLD_DB) {
    publish_avg(avg);
  } else {
    publish_raw();
  }
}

static void
noise_processing() {
 
  cfs_read(fd, buf, sizeof(buf));
  char *token;
  LOG_INFO("%s", buf);
  const char delim[2] =",";
  token = strtok(buf, delim);
  noise_values[position] =token - '0';
  token = strtok(NULL, delim);
  x =token - '0';
  token = strtok(NULL, delim);
  y =token - '0';
  token = strtok(NULL, delim);
  region =token - '0';
  printf("Noise lvl: %d dB\n", noise_values[position]);

  publish_noise();
  position = (position + 1) % MAX_WINDOW_SIZE;
}
/*---------------------------------------------------------------------------*/
static void
connect_to_broker(void)
{
  mqtt_connect(&conn, conf.broker_ip, conf.broker_port,
               DEFAULT_KEEP_ALIVE_INTERVAL);

  state = STATE_CONNECTING;
}
/*---------------------------------------------------------------------------*/
static void
mqtt_state_machine()
{
  switch(state) {
  case STATE_INIT:
    /* If we have just been configured register MQTT connection */
    mqtt_register(&conn, &noise_simulated_sensor_process, client_id, mqtt_event,
                  MAX_TCP_SEGMENT_SIZE);

    mqtt_set_username_password(&conn, "use-token-auth",
                                   conf.auth_token);

    /* _register() will set auto_reconnect; we don't want that */
    conn.auto_reconnect = 0;
    connect_attempt = 1;

    state = STATE_REGISTERED;
    LOG_INFO("Init\n");
    /* Continue */
  case STATE_REGISTERED:
    if(uip_ds6_get_global(ADDR_PREFERRED) != NULL) {
      /* Registered and with a global IPv6 address, connect! */
      LOG_INFO("Joined network! Connect attempt %u\n", connect_attempt);
      connect_to_broker();
    }
    etimer_set(&mqtt_timer, NET_CONNECT_PERIODIC);
    return;
    break;
  case STATE_CONNECTING:
    /* Not connected yet. Wait */
    LOG_INFO("Connecting: retry %u...\n", connect_attempt);
    break;
  case STATE_CONNECTED:
    /* If the timer expired, the connection is stable */
    if(timer_expired(&connection_life)) {
      /*
       * Intentionally using 0 here instead of 1: We want RECONNECT_ATTEMPTS
       * attempts if we disconnect after a successful connect
       */
      connect_attempt = 0;
    }

    if(mqtt_ready(&conn) && conn.out_buffer_sent) {
      /* Connected; sampling */
      LOG_INFO("Noise sampling started\n");

      radio_set_channel(NOISE_CHANNEL);
      state = STATE_SAMPLING;
      etimer_set(&mqtt_timer, 0.1 * CLOCK_SECOND);
      /* Return here so we don't end up rescheduling the timer */
      return;
    } else {
      /*
       * Our publish timer fired, but some MQTT packet is already in flight
       * (either not sent at all, or sent but not fully ACKd)
       *
       * This can mean that we have lost connectivity to our broker or that
       * simply there is some network delay. In both cases, we refuse to
       * trigger a new message and we wait for TCP to either ACK the entire
       * packet after retries, or to timeout and notify us
       */
      LOG_INFO("Publishing... (MQTT state=%d, q=%u)\n", conn.state,
          conn.out_queue_full);
    }
    break;
  case STATE_SAMPLING:
    noise_processing();
    state = STATE_CONNECTED;
    etimer_set(&mqtt_timer, conf.pub_interval);
    return;
  case STATE_DISCONNECTED:
    LOG_INFO("Disconnected\n");
    if(connect_attempt < RECONNECT_ATTEMPTS ||
       RECONNECT_ATTEMPTS == RETRY_FOREVER) {
      /* Disconnect and backoff */
      clock_time_t interval;
      mqtt_disconnect(&conn);
      connect_attempt++;

      interval = connect_attempt < 3 ? RECONNECT_INTERVAL << connect_attempt :
        RECONNECT_INTERVAL << 3;

      LOG_INFO("Disconnected: attempt %u in %lu ticks\n", connect_attempt, interval);

      etimer_set(&mqtt_timer, interval);

      state = STATE_REGISTERED;
      return;
    } else {
      /* Max reconnect attempts reached; enter error state */
      state = STATE_ERROR;
      LOG_ERR("Aborting connection after %u attempts\n", connect_attempt - 1);
    }
    break;
  case STATE_CONFIG_ERROR:
    /* Idle away. The only way out is a new config */
    LOG_ERR("Bad configuration.\n");
    return;
  case STATE_ERROR:
  default:
    /*
     * 'default' should never happen
     *
     * If we enter here it's because of some error. Stop timers. The only thing
     * that can bring us out is a new config event
     */
    LOG_INFO("Default case: State=0x%02x\n", state);
    return;
  }

  /* If we didn't return so far, reschedule ourselves */
  etimer_set(&mqtt_timer, STATE_MACHINE_PERIODIC);
}

static void
init_noise_values(void) {
  position = 0;

  for (size_t i = 0; i < MAX_WINDOW_SIZE; i++) {
    noise_values[i] = 0;
  }
}

static void
init_file_reading(void) {
cfs_close(fd);
fd = cfs_open("asdasfasrbisdfdsbgiusrgt" ,CFS_READ);
LOG_INFO("%d\n", fd);
 if(fd < 0) {
		LOG_WARN("Failed to open");
 }
 else{
    LOG_INFO("File opened\n");
    cfs_seek(fd, 0, CFS_SEEK_SET);
    LOG_INFO("Seek done\n");
    cfs_read(fd, buf, sizeof(message));
    LOG_INFO("READ\n");
    LOG_INFO("%s\n", buf);
 }

}

PROCESS_THREAD(noise_simulated_sensor_process, ev, data)
{
  PROCESS_BEGIN();

  init_noise_values();
  init_file_reading();
  init_config();
  update_config();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&mqtt_timer));
    mqtt_state_machine();
  }
  cfs_close(fd);
  printf("Done\n");

  PROCESS_END();
}
