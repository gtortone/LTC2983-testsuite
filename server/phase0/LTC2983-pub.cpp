#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <math.h>
#include <sys/time.h>
#include <zmq.hpp>

#include <string>
#include <vector>

#include "spiLTC2983.h"

int main(int argc, char **argv) {

   int channel = 0, rejection = 0;
   int valid_opt = 0;

   char rej_label[][32] = { "50Hz", "60Hz", "50Hz and 60Hz" };

   uint32_t chdata, nsamples = 0;
   float fval = 0;
   int opt;
   extern unsigned char fault_code;
  
   int option_index = 0;
   static struct option long_options[] = {
      {"channel",      required_argument, 0, 0},
      {"rejection",    required_argument, 0,  0 },
      {0,         0,                 0,  0 }
   };

   while ( (opt = getopt_long(argc, argv, "c:r:", long_options, &option_index)) != -1 ) {  // for each option...

      switch ( opt ) {
         case 'c':
            channel = atoi(optarg);
   	    if( (channel != 2) && (channel != 4) && (channel != 6) && (channel != 16) && (channel != 18) && (channel != 20)) {
               printf("ERROR: wrong channel number\n");
               exit(-1);
            }
            valid_opt++;
            break;
         case 'r':
 	    rejection = atoi(optarg);
            if( (rejection != 0) && (rejection != 1) && (rejection != 2) ) {
               printf("ERROR wrong rejection frequency\n");
               exit(-1);
            }
            valid_opt++;
            break;
        }
   } 

   if( (argc < 2) || (valid_opt != 2) ){
      printf("\nUSAGE: LTC2499-pub -c <ch> -r <rej_freq>\n\n");
      printf("channel num:    2, 4, 6, 16, 18, 20\n");
      printf("rejection freq: 2=50Hz, 1=60Hz, 0=50Hz and 60Hz\n\n");
      exit(0);
   }

   if(LTC_SPI_init(1, 0) == false) {
      printf("LTC_SPI_init: error\n");
      abort();
   }

   chdata = (uint32_t) SENSOR_TYPE__DIRECT_ADC | (uint32_t) DIRECT_ADC_DIFFERENTIAL;

   // ----- Channel 2: Assign Direct ADC -----
   // 1) conn J1 - pin 12  2) conn J1 - pin 9	GND = pin 14
   LTC_ch_config(2, chdata);
   LTC_ch_add(2);

   // ----- Channel 4: Assign Direct ADC -----
   LTC_ch_config(4, chdata);
   LTC_ch_add(4);
 
   // ----- Channel 6: Assign Direct ADC -----
   LTC_ch_config(6, chdata);
   LTC_ch_add(6);

   // ----- Channel 16: Assign Direct ADC -----
   LTC_ch_config(16, chdata);
   LTC_ch_add(16);

   // ----- Channel 18: Assign Direct ADC -----
   LTC_ch_config(18, chdata);
   LTC_ch_add(18);

   // ----- Channel 20: Assign Direct ADC -----
   LTC_ch_config(20, chdata);
   LTC_ch_add(20);

   // configure global parameters
   LTC_reg_write(0xF0, (uint8_t)(TEMP_UNIT__C | rejection));
   LTC_reg_write(0xFF, (uint8_t)(0));	// set 0ms mux delay between conversions

   zmq::context_t context (1);
   zmq::socket_t publisher (context, ZMQ_PUB);
   publisher.bind("tcp://eth0:5556");

   printf("\n>>> DIRECT ADC\n");
   printf(">>> channel: %d\n", channel);
   printf(">>> rejection: %s\n", rej_label[rejection]);
   printf(">>> ZMQ publisher on port 5556\n");
   printf("\n");

   while(1) {
    
      LTC_ch_convert(2);
      fval = LTC_voltage_read(2);
      //printf("READ direct ADC ch2 (float): %f\n", fval);

      // convert V to uV
      fval = fval * 1000.0 * 1000.0;

      nsamples++;
      printf("   Total ADC samples: %u  last sample: %f uV    fault code: %d   \r", nsamples, fval, fault_code);
      fflush(stdout);

      zmq::message_t message(20);
      sprintf((char *) message.data(), "%f", fval);
      publisher.send(message);
   }

   printf("\n\n");

   LTC_SPI_close();   

   return(0);
}
