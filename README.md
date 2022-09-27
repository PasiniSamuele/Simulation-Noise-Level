# Simulation and Analysis of Noise Level 
The designed system studies the level of noise in different regions of a country. The noise is studied
in different ways according to the region:
- IoT devices: in some regions the sensor data is assumed to be available, so it is possible
to collect data with IoT sensors able to measure the noise level according to the distance to
noise sources. To simplify computation in this scenario, we assume that a sensor detects the
noise level only from its closest noise source and the noise level detected depends only on the
distance between the source and the device.
- Virtual Sensors: in some regions sensor data are not available, so the noise level detection
is simulated via virtual sensors, that read data coming from an existing dataset of noise level
readings.
- Computer Simulation: other regions rely on simulations based on population dynamics.
In this scenario the noise level is computed for every squared meter of the region considering
a certain number of people and vehicles moving in the region.
Data collected in these ways are sent to the backend, where there are data cleaning and enrichment
operations.
Incomplete or invalid data are discarded and each reading is associated with the nearest point
of interest. We consider data as invalid if the noise level is lower than 10 dB or greater than
180 dB. Data are also invalid if the region ID or the coordinates are unset or the location of the
measurement is outside the boundaries of the region.
