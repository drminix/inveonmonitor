#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define OUTPUT_DIRECTORY "/local_scratch/slee91"


//projection
#define NUMBER_OF_PMT 9
#define PMT_WIDTH     24
#define PMT_HEIGHT    24
#define NUMBER_OF_HEAD 2

#define INVEON_HEIGHT 72
#define INVEON_WIDTH 72


//GATE server specific
#define UTMC_NUMBER 1
#define UTMC_CORE 63

//hydra cluster specific
#define HYDRA_NUMBER 31
#define HYDRA_CORE 7  //take out 1 in case other user is using the machine

//arc cluster specific
#define ARC_NUMBER 16
#define ARC_CORE 3 //take out 1 in case other user is using the machine

//To be incorporated into the systtem
#define NETWORK_CONFIGURATION network.txt

//default location
#define DEFAULT_LOCATION "/research/jgregor/slee91/dropbox/Dropbox/samlee/inveon/"


#endif // CONFIGURATION_H
