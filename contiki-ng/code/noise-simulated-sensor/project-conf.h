/*---------------------------------------------------------------------------*/
/**
 * \file
 * Project specific configuration defines for the MQTT UTIL
 */
/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/
/* Enable TCP */
#define UIP_CONF_TCP 1
/*---------------------------------------------------------------------------*/
/* User configuration */
/*---------------------------------------------------------------------------*/
#define MQTT_PUBLISH_TOPIC  "simulated-noise"
#define MQTT_BROKER_IP_ADDR "fd00::1"
//*---------------------------------------------------------------------------*/
#define IEEE802154_CONF_DEFAULT_CHANNEL      21
//*---------------------------------------------------------------------------*/
#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/
/** @} */