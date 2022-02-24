#include "contiki.h"
#include "net/netstack.h"
#include "dev/radio.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mqtt-util.h"

#define READ_NOISE_TIMER 10 * CLOCK_SECOND

#define MAX_WINDOW_SIZE 6
#define AVG_THRESHOLD_DB 70


static uint16_t noise_values[MAX_WINDOW_SIZE];
/*---------------------------------------------------------------------------*/
PROCESS(noise_sensor_process, "Noise sensor process");
AUTOSTART_PROCESSES(&noise_sensor_process);
/*---------------------------------------------------------------------------*/

static void
noise_processing(uint16_t position) {
  radio_value_t value;
  radio_result_t rv;

  rv = NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &value);

  if (rv == RADIO_RESULT_OK) {
    noise_values[position] = (uint16_t) value + 110;  
    printf("Noise lvl: %d dB\n", noise_values[position]);
    
    send_noise();
  } else {
    printf("Something went wrong...");
  }

  etimer_set(&noise_timer, READ_NOISE_TIMER);
}

static void
send_noise(void) {
  double avg = 0;
  
  for (size_t i = 0; i < MAX_WINDOW_SIZE; i++) {
    avg += noise_values[i];
  }
  
  avg /= MAX_WINDOW_SIZE;
  
  if (avg < AVG_THRESHOLD_DB) {
    send_avg(avg);
  } else {
    send_raw();
  }
}


static void
send_avg(double avg) {
  
}

static void
send_raw(void) {
  
}

static void
init_noise_values(void) {
  for (size_t i = 0; i < MAX_WINDOW_SIZE; i++) {
    noise_values[i] = 0;
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(noise_sensor_process, ev, data)
{
  static struct etimer noise_timer;
  static struct etimer mqtt_timer;

  PROCESS_BEGIN();
  
  etimer_set(&noise_timer, READ_NOISE_TIMER);
  init_noise_values();

  /* Initialize MQTT */
  LOG_INFO("MQTT Noise Process\n");
  init_config();
  update_config(&mqtt_timer);

  static uint16_t position = 0;
  
  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&mqtt_timer));

    mqtt_state_machine(&mqtt_timer);

    if (etimer_expired(&noise_timer)) {
      noise_processing(position);
      position = (position + 1) % MAX_WINDOW_SIZE;
    }
  }

  printf("Done\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
