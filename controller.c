#include <wiringPi.h>
#include <lcd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

//USE WIRINGPI PIN NUMBERS
#define LCD_RS  25               //Register select pin
#define LCD_E   24               //Enable Pin
#define LCD_D4  23               //Data pin 4
#define LCD_D5  22               //Data pin 5
#define LCD_D6  21               //Data pin 6
#define LCD_D7  14               //Data pin 7
#define MAXTIMINGS 85
#define DHTPIN 7
#define RelayPn 0
#define RelayPn_h 1


int lcd;
int dht11_dat[5] = {0, 0, 0, 0, 0};

void read_dht11_dat()
{
        uint8_t laststate = HIGH;
        uint8_t counter = 0;
        uint8_t j = 0, i;
        float f,humidity; 

        dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

        pinMode(DHTPIN, OUTPUT);
        digitalWrite(DHTPIN, LOW);
        delay(18);
        
        digitalWrite(DHTPIN, HIGH);
        delayMicroseconds(40);
        
        pinMode(DHTPIN, INPUT);

        for (i = 0; i < MAXTIMINGS; i++)
        {
                counter = 0;
                while (digitalRead(DHTPIN) == laststate)
                {
                        counter++;
                        delayMicroseconds(1);
                        if (counter == 255)
                        {
                                break;
                        }
                }
                laststate = digitalRead(DHTPIN);

                if (counter == 255)
                        break;

                if ((i >= 4) && (i % 2 == 0))
                {
                        dht11_dat[j / 8] <<= 1;
                        if (counter > 16)
                                dht11_dat[j / 8] |= 1;
                        j++;
                }
         }

        if ((j >= 40) && (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF)))
        {
                f = dht11_dat[2] * 9. / 5. + 32 ;
                f = dht11_dat[2] ;

                lcdPosition(lcd, 0, 0);
                lcdPrintf(lcd, "Humidity: %d.%d %%\n", dht11_dat[0], dht11_dat[1]);

                lcdPosition(lcd, 0, 1);
                lcdPrintf(lcd, "Temp: %d.0 C", dht11_dat[2]); //Uncomment for Celsius
                //lcdPrintf(lcd, "Temp: %f.%f %% C", f); //Comment out for Celsius

		humidity = dht11_dat[0]+(dht11_dat[1]/10);
        if(dht11_dat[2]<10)
	{ 
		printf("TEMPERATURE LOW/n");
		digitalWrite(RelayPn, LOW); 
	}
         else if(dht11_dat[2] > 28)
	{ 
		//lcdPosition(lcd, 0, 3);
		printf("TEMPERATURE HIGH\n");
		digitalWrite(RelayPn, HIGH); 
	}

	if(humidity<50) {
		digitalWrite(RelayPn_h, LOW); 
        } else if(humidity > 80) {
		digitalWrite(RelayPn_h, HIGH);
	}
}

void read_ip_dat()
{

    int n;
    struct ifreq ifr;
    char array[] = "wlan0";
 
    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);
    //display result
    lcdPosition(lcd, 0, 2);
    lcdPrintf(lcd, "IP%s" , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );
    //return 0;

}


int main(void)
{
        int lcd;
        wiringPiSetup();
        lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);

        while (1)
        {
                read_dht11_dat();
                read_ip_dat();
                delay(1000); 
        }

        return(0);
}

