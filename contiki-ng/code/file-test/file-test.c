#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>


#define FILENAME "test1.csv"

static int32_t fd;
static char buf[64];
static char message[32];
/*static char* x;
static char* y;
static char* region;*/


PROCESS(file_test_process, "Noise simulated sensor process");
AUTOSTART_PROCESSES(&file_test_process);


static void
init_file_reading(void) {
/*struct file * file = find_file("asdasfasrbisdfdsbgiusrgt");
if(file ==NULL){
  	LOG_WARN("File null");
}*/
//cfs_close(fd);
fd = cfs_open(FILENAME ,CFS_READ);
printf("%d\n", fd);
 if(fd < 0) {
		printf("Failed to open");
 }
 else{
    printf("File opened\n");
    cfs_seek(fd, 0, CFS_SEEK_SET);
    printf("Seek done\n");
    cfs_read(fd, buf, sizeof(message));
    printf("READ\n");
    printf("%s\n", buf);
 }

}

PROCESS_THREAD(file_test_process, ev, data)
{
  PROCESS_BEGIN();
  init_file_reading();

  static struct etimer timer;


  /* Setup a periodic timer that expires after 2 seconds. */
  etimer_set(&timer, CLOCK_SECOND * 2);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  }
  printf("Done\n");

  PROCESS_END();
}
