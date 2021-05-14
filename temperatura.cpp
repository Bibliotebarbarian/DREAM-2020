// C library headers
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <cstdlib>

int main() {
	char* buff=NULL;
	char* garbage=NULL;
	//char date[128];
	//time_t rawtime;
	FILE *tempFile;
	long unsigned int buffsize=0;
	
	if( (tempFile=fopen("temp.dat", "a")) == NULL){
		printf("ERRORE APERTURA FILE \n");
	}
	
	
  	// Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
  	int serial_port = open("/dev/ttyACM0", O_RDWR);
  	FILE* serial_stream= fdopen(serial_port, "r");

  	// Create new termios struc
  	struct termios config;
    // Read in existing settings, and handle any error
  	if(tcgetattr(serial_port, &config) != 0) {
    	printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	  	return 1;
  	}
  
  
	//Set options
	cfmakeraw(&config);
	config.c_cflag |= (CLOCAL | CREAD);
	config.c_iflag &= ~(IXOFF | IXANY);
	config.c_cc[VMIN] = sizeof(buff);
	config.c_cc[VTIME] = 2;
	cfsetispeed(&config, B9600);
	cfsetospeed(&config, B9600);
	
	
	// Save config settings, also checking for error
  	if (tcsetattr(serial_port, TCSANOW, &config) != 0) {
    	printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
  		return 1;
	}
	
	//int num_bytes = read(serial_port, &buff, sizeof(buff));

	//getline(&garbage, &buffsize, serial_stream);
	buffsize = 0;
	getline(&buff, &buffsize, serial_stream);
	//buff = "27.2\r\n";  //DEBUG
	
	while( !(isdigit(buff[0]) && isdigit(buff[1]) && buff[2]==46 && isdigit(buff[3]) && buff[4]==13 && buff[5]== 10) ){
	buffsize=0;
	getline(&buff, &buffsize, serial_stream);
	}
	
	// n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
	/*if (num_bytes < 0) {
    		printf("Error reading: %s", strerror(errno));
      		return 1;
	}*/

	//time(&rawtime);
	//const tm* dataFormat = localtime(&rawtime);
	//strftime(date, sizeof(date), "%m/%d\t%H:%M:%S", dataFormat);//ora e data
	
	

	fprintf(tempFile, "%s", buff);

	fclose(tempFile);
	return 0;
}
