/*
 * CS can be configured in hw_enc28j60.h
 * Configure mymac and myip below
 */

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "ip_arp_udp_tcp.h"
#include "avr_compat.h"
#include "net.h"
#include "aux_globals.h"
#include "hw_enc28j60.h"
#include "hw_ds18b20.h"
#include "hw_dht11.h"


// enc28j60 Ethernet Class

// please modify the following two lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
static uint8_t mymac[6] = {0x14,0x15,0x16,0x17,0x18,0x19};
static uint8_t myip[4] = {192,168,1,24};
// listen port for tcp/www (max range 1-254)
#define MYWWWPORT 80
// listen port for udp
#define MYUDPPORT 1200
// working buffer
#define BUFFER_SIZE 550
static uint8_t buf[BUFFER_SIZE+1];
// Global counters
//static int nPingCount = 0, nAccessCount = 0;
// Objects

int ds18b20_temp = 0;
int		milis = 0,sec = 0,
// sensor data
dht11_temp=0, dht11_humidity=0;
static char szS1[100] = {0};
	

// the password string (only the first 5 char checked), (only a-z,0-9,_ characters):
static char password[]="secret"; // must not be longer than 9 char


uint8_t verify_password(char *str)
{
	// the first characters of the received string are
	// a simple password/cookie:
	if (strncmp(password,str,5)==0){
		return(1);
	}
	return(0);
}

// takes a string of the form password/commandNumber and analyse it
// return values: -1 invalid password, otherwise command number
//                -2 no command given but password valid
//                -3 valid password, no command and no trailing "/"
int8_t analyse_get_url(char *str)
{
	uint8_t loop=1;
	uint8_t i=0;
	while(loop){
		if(password[i]){
			if(*str==password[i]){
				str++;
				i++;
				}else{
				return(-1);
			}
			}else{
			// end of password
			loop=0;
		}
	}
	// is is now one char after the password
	if (*str == '/'){
		str++;
		}else{
		return(-3);
	}
	// check the first char, garbage after this is ignored (including a slash)
	if (*str < 0x3a && *str > 0x2f){
		// is a ASCII number, return it
		return(*str-0x30);
	}
	return(-2);
}

// answer HTTP/1.0 301 Moved Permanently\r\nLocation: password/\r\n\r\n
// to redirect to the url ending in a slash
uint16_t moved_perm(uint8_t *buf)
{
	uint16_t plen;
	plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 301 Moved Permanently\r\nLocation: "));
	plen=fill_tcp_data(buf,plen,password);
	plen=fill_tcp_data_p(buf,plen,PSTR("/\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<h1>301 Moved Permanently</h1>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("add a trailing slash to the url\n"));
	return(plen);
}


// prepare the webpage by writing the data to the tcp send buffer
uint16_t print_webpage(uint8_t *buf,uint8_t on_off)
{
	uint16_t plen;
	
	sprintf(szS1, "Sensors:<br><li>DS18B20 Temp: %d°C</li><li>DHT-11 Temp: %d°C Humidity: %dRH</li>", ds18b20_temp,dht11_temp, dht11_humidity);
	
	
	plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));
	
	plen=fill_tcp_data(buf,plen,szS1);
	plen=fill_tcp_data_p(buf,plen,PSTR("<li><p>Output is: "));
	if (on_off){
		plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#00FF00\"> ON</font></li>"));
		}else{
		plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#FF0000\"> OFF</font></li>"));
	}
	plen=fill_tcp_data_p(buf,plen,PSTR("<center></p>\n<p><a href=\"."));
	if (on_off){
		plen=fill_tcp_data_p(buf,plen,PSTR("/0\">Switch off</a><p>"));
		}else{
		plen=fill_tcp_data_p(buf,plen,PSTR("/1\">Switch on</a><p>"));
	}
	plen=fill_tcp_data_p(buf,plen,PSTR("<small><a href=\".\">[refresh status]</a></small>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</center><hr>ATmega8 Temperature & Humidity Server By Farid Ghoorchian\n"));
	
	return(plen);
}

	
 /*
	timer0 overflow interrupt
	event to be exicuted every  1.024ms here
*/
ISR (TIMER0_OVF_vect)  
{
	milis ++;
	if (milis >= 976) {
		 sec++;
		 milis = 0; // 976 x 1.024ms ~= 1000 ms = 1sec
		
 
		if (sec%5 == 0) { //every 5 seconds
			// read sensors //
			
			// ds18b20
			ds18b20_temp = therm_read_temperature();
			// dht-11
			int tmp_humi = 0 , tmp_temp = 0;
			if (DHT11_read(&tmp_temp, &tmp_humi) == 0) {
				dht11_temp = tmp_temp;
				dht11_humidity = tmp_humi;
				
			}
			
			
		}	
		
		if (sec%60 ==0) {
			sec = 0;
		}
	}		

		
}


/*
 * Main entry point
 */
int main(void) {
	// 2.CREATE Timer T0 to count seconds
	//
	TIMSK |= (1 << TOIE0);
	// set prescaler to 64 and start the timer
	TCCR0 |= (1 << CS01) | (1 << CS00);
	// start timer and interrupts
	sei();

	//=====setup eth interface
	uint16_t plen = 0,  dat_p = 0;
	
	uint8_t cmd_pos=0;
	int8_t cmd;
	uint8_t payloadlen=0;
	char str[30];
	char cmdval;

	//initialize enc28j60
    enc28j60Init(mymac);
	fcpu_delay_ms(100);
        
    // Magjack leds configuration, see enc28j60 datasheet, page 11 
	enc28j60PhyWrite(PHLCON,0x476);

	fcpu_delay_ms(100);
	
	 // LED
	 /* enable PB1, LED as output */
	 DDRD|= (1<<DDD1);

	 /* set output to Vcc, LED off */
	 PORTD|= (1<<PORTD1);

	 // the transistor on PD7
	 DDRD|= (1<<DDD7);
	 PORTD &= ~(1<<PORTD7);// transistor off
	 
	 
	 

    //init the ethernet/ip layer:
    init_ip_arp_udp_tcp(mymac,myip,MYWWWPORT);
	fcpu_delay_ms(100);
	
	//char temp[200]={0};
		
    while(1){
		// get the next new packet:
                plen = enc28j60PacketReceive(BUFFER_SIZE, buf);

                /*plen will ne unequal to zero if there is a valid 
                 * packet (without crc error) */
                if(plen==0){
                        continue;
                }
                        
                // arp is broadcast if unknown but a host may also
                // verify the mac address by sending it to 
                // a unicast address.
                if(eth_type_is_arp_and_my_ip(buf,plen)){
                        make_arp_answer_from_request(buf);
                        continue;
                }

                // check if ip packets are for us:
                if(eth_type_is_ip_and_my_ip(buf,plen)==0){
                        continue;
                }
                // led----------
                
                
                if(buf[IP_PROTO_P]==IP_PROTO_ICMP_V && buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V){
                        // a ping packet, let's send pong
                        make_echo_reply_from_request(buf,plen);
                        continue;
                }
                // tcp port www start, compare only the lower byte
                if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==0&&buf[TCP_DST_PORT_L_P]==MYWWWPORT){
                        if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V){
                                make_tcp_synack_from_syn(buf);
                                // make_tcp_synack_from_syn does already send the syn,ack
                                continue;
                        }
                        if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V){
                                init_len_info(buf); // init some data structures
                                // we can possibly have no data, just ack:
                                dat_p=get_tcp_data_pointer();
                                if (dat_p==0){
                                        if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V){
                                                // finack, answer with ack
                                                make_tcp_ack_from_any(buf);
                                        }
                                        // just an ack with no data, wait for next packet
                                        continue;
                                }
                                if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
                                        // head, post and other methods:
                                        //
                                        
                                        plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
                                        goto SENDTCP;
                                }
                                if (strncmp("/ ",(char *)&(buf[dat_p+4]),2)==0){
                                        plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"));
                                        plen=fill_tcp_data_p(buf,plen,PSTR("<p>Usage: http://host_or_ip/password</p>\n"));
                                        goto SENDTCP;
                                }
                                cmd=analyse_get_url((char *)&(buf[dat_p+5]));
                                //  
                                // 
                                if (cmd==-1){
                                        plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
                                        goto SENDTCP;
                                }
                                if (cmd==1){
                                        PORTD|= (1<<PORTD7);// transistor on
                                }
                                if (cmd==0){
                                        PORTD &= ~(1<<PORTD7);// transistor off
                                }
                                if (cmd==-3){
                                        // redirect to add a trailing slash
                                        plen=moved_perm(buf);
                                        goto SENDTCP;
                                }
                                // if (cmd==-2) or any other value
                                // just display the status:
                                plen=print_webpage(buf,(PORTD & (1<<PORTD7)));
                                //
SENDTCP:
                                make_tcp_ack_from_any(buf); // send ack for http get
                                make_tcp_ack_with_data(buf,plen); // send data
                                continue;
                        }

                }
                // tcp port www end
                //
                // udp start, we listen on udp port 1200=0x4B0
                if (buf[IP_PROTO_P]==IP_PROTO_UDP_V&&buf[UDP_DST_PORT_H_P]==4&&buf[UDP_DST_PORT_L_P]==0xb0){
                        payloadlen=buf[UDP_LEN_L_P]-UDP_HEADER_LEN;
                        // you must sent a string starting with v
                        // e.g udpcom version 10.0.0.24
                        if (verify_password((char *)&(buf[UDP_DATA_P]))){
                                // find the first comma which indicates 
                                // the start of a command:
                                cmd_pos=0;
                                while(cmd_pos<payloadlen){
                                        cmd_pos++;
                                        if (buf[UDP_DATA_P+cmd_pos]==','){
                                                cmd_pos++; // put on start of cmd
                                                break;
                                        }
                                }
                                // a command is one char and a value. At
                                // least 3 characters long. It has an '=' on
                                // position 2:
                                if (cmd_pos<2 || cmd_pos>payloadlen-3 || buf[UDP_DATA_P+cmd_pos+1]!='='){
                                        strcpy(str,"e=no_cmd");
                                        goto ANSWER;
                                }
                                // supported commands are
                                // t=1 t=0 t=?
                                if (buf[UDP_DATA_P+cmd_pos]=='t'){
                                        cmdval=buf[UDP_DATA_P+cmd_pos+2];
                                        if(cmdval=='1'){
                                                PORTD|= (1<<PORTD7);// transistor on
                                                strcpy(str,"t=1");
                                                goto ANSWER;
                                        }else if(cmdval=='0'){
                                                PORTD &= ~(1<<PORTD7);// transistor off
                                                strcpy(str,"t=0");
                                                goto ANSWER;
                                        }else if(cmdval=='?'){
                                                if (PORTD & (1<<PORTD7)){
                                                        strcpy(str,"t=1");
                                                        goto ANSWER;
                                                }
                                                strcpy(str,"t=0");
                                                goto ANSWER;
                                        }
                                }
                                strcpy(str,"e=no_such_cmd");
                                goto ANSWER;
                        }
                        strcpy(str,"e=invalid_pw");
ANSWER:
                        make_udp_reply_from_request(buf,str,strlen(str),MYUDPPORT);
                }
        } // while
    return (0);
} 