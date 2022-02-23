#include "contiki.h"
#include "net/netstack.h"
#include "dev/radio.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TRIGGER_TIME 3 * CLOCK_SECOND

static radio_value_t value;
/*---------------------------------------------------------------------------*/
PROCESS(noise_sensor_process, "Noise sensor process");
AUTOSTART_PROCESSES(&noise_sensor_process);
/*---------------------------------------------------------------------------*/

static void
noise_processing(void) {
  radio_result_t rv;

  rv = NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &value);
  
  uint16_t noise = (uint16_t) value + 110;  

  printf("RSSI: ");
  if(rv == RADIO_RESULT_OK) {
    printf("%d dB\n", noise);
  } else {
    printf("Something went wrong...");
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(noise_sensor_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();
  
  etimer_set(&et, TRIGGER_TIME);
  
  for(;;) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    
    noise_processing();
    
    etimer_set(&et, TRIGGER_TIME);
  }



  printf("Done\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
