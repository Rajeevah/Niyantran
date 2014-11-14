#define F_CPU 8000000UL

// THE INCLUDE FILES

#include <avr/io.h>
#include <util/delay.h>
#include<avr/interrupt.h>

#include<c:\PROJECT\fns.h>
#include<c:\PROJECT\our_header_file.h>
#include<c:\PROJECT\fn_prototypes.h>



//PIN ASSIGNMENTS

#define dataout PORA.bit0
#define clockout PORA.bit1
#define datain PINNA.bit2                  // these are the pin assignments used for communicating with the 
#define clockin PINNA.bit3		//host



//CLOCK STATES AS SHOWN IN DIAGRAM	 			    1        3
#define RISE 0	//		               			   ______2	_____......
#define HIGH 1          // clock state variables          	0 |      |_____|
#define FALL 2
#define LOW 3


// some boolean states

#define TRUE 			1
#define FALSE			0
#define ENABLED			1
#define DISABLED 		0

// PS/2 command set

#define MOUSE_ID		0X00
#define RESET 			0XFF
#define RESEND 			0XFE
#define ERROR 			0XFC
#define ACK			0XFA
#define BAT			0XAA
#define SET_DEFAULT		0XF6    //SAMPLING RATE= 100 SAMPLES/SEC, RESOLUTION=4 COUNTS/MM,SCALING=1:1
#define DISABLE_DATA_REPORTING	0XF5
#define ENABLE_DATA_REPORTING	0XF4
#define SET_SAMPLE_RATE		0XF3
#define GET_DEVICE_ID 		0XF2
#define SET_REMOTE_MODE		0XF0
#define SET_WARP_MODE		0XEE
#define RESET_WARP_MODE		0XEC
#define READ_DATA		0XEB
#define SET_STREAM_MODE		0XEA
#define STATUS_REQUEST		0XE9
#define SET_RESOLUTION		0XE8
#define SET_SCALING21 		0xE7
#define SET_SCALING11		0xE6

//FLAGS and bytes USED

unsigned char clock_state;
unsigned char bit_index;
unsigned char transmit_byte;
unsigned char next_clock_state;
unsigned char byte_parity;
unsigned char txn,rxn;				
unsigned char receive_byte,receive_bit;
unsigned char resend;
unsigned char sample_rate, resolution,scaling;
unsigned char previous_byte;
unsigned char mouse_state;
unsigned char data_reporting;
unsigned char wait_for_res;
unsigned char wait_for_SR;
unsigned char mouse_delay,mouse_count;
unsigned char gen_mouse_packet;
unsigned char B1,B2,B3,stream_mode,warp_mode,remote_mode;
void manip(unsigned char x);

//ARRAY DECLARARTION
#define msg_array_len 100
unsigned char msg_array[msg_array_len];
unsigned char msg_array_full;
unsigned char msg_array_empty;
unsigned char msg_array_in_pointer=0;
unsigned char msg_array_out_pointer; 
unsigned char current_byte;

unsigned char rec_par,x;


// MOUSE STATES   

unsigned char mouse_state;
#define IDLE 0
#define ABORT 1
#define BUSY 2
#define REQUEST_WAITING 3
#define REQUEST 4


//beginning of main


int main(void) {
	 

	DDRA  = 0xF3;
	PORTA = 0xF3;
	DDRB  = 0x00;
	DDRC  = 0xFF;
	DDRD  = 0xff;
	PORTD = 0xFF;

	_delay_ms(500);
	
 	msg_array_insert(BAT);
	msg_array_insert(MOUSE_ID);

	

	sei();
	OCR0=19;
	TIMSK=2;
	//Timer/Counter Control Register 
	TCCR0=0x0A;


  while(1) {
		data_reporting=1;
		
		if(data_reporting==1)
		{	PORD.bit7=0;			// data reporting enabled
			get();
		}

	  }
  return 0;

}


ISR (TIMER0_COMP_vect)

{
	
	PORD.bit0=0;		// interrupt enter 	
	if (mouse_delay	>0)
		mouse_delay--;
//msg_array_insert(0xaa);	

	
	if(txn==1 && mouse_delay==0 ) 		//start of device to host commn
	{				
		
		if( bit_index==0 && current_byte==0 && msg_array_empty==0)
		{

			transmit_byte=msg_array_get();
			//transmit_byte=0xaa;
			byte_parity=get_parity(transmit_byte);
			current_byte=1;
			mouse_delay=7;	
			clockout=1;		
			
		}

	


		clock_state=next_clock_state;
	
		switch(clock_state)   // this switch ensure that the host reads a bit at falling edge of clock 
		{
	
			case RISE:
					clockout=1;
					if(bit_index<11) 
						next_clock_state=HIGH;
					else
					{
						txn=0;
						bit_index=0;
						
					}

					
				break;		// end of rise

			case HIGH:
				
					switch(bit_index)
					{
						case 0:
							dataout=0;
							break;
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
						case 7:
						case 8:
							dataout=transmit_byte & 0x01;
							transmit_byte = transmit_byte >>1;
							break;
				
						case 9: dataout=byte_parity;
							break;
						case 10:dataout=1;
								current_byte=0;
							break;
						default:
							break;
					}
					bit_index++;

					next_clock_state=FALL;
				break;
				   //end of high
		
			case FALL:
				clockout=0;
				
				next_clock_state=LOW;
				break;			//end of fall
			case LOW:
				next_clock_state=RISE;
				
				break;
	
			default:
			break;

	
		
		}			//	end of device to host code

	
	
	}


if(rxn==1)			//start of host to device commn
	{	
		clock_state=next_clock_state;
	
		switch(clock_state)			// this switch ensure that device reads a bit at rising edge of clock 
		{
			case RISE:
				clockout=1;
			
				if(bit_index>=0 && bit_index<=7)	//receive
				{
				
					receive_bit  = datain;
					receive_byte = ( (receive_bit & 0x01) << bit_index)|receive_byte;
					
				}
		
				if(bit_index==8)
				{
					rec_par=datain;
				}

				if(bit_index==9)
				{
					x=datain;	//stop
				}

				if(bit_index<11)
				{
					next_clock_state=HIGH;
					bit_index++;
				}

				break;
			case HIGH:
				switch(bit_index)
				{
			
					case 10:
						dataout=0;  // ack for rxn
						break;
					case 11:
						dataout=1;  // stop
		
						rxn=0;
						bit_index=0;
						process_cmd(receive_byte);

						receive_bit=0;
						receive_byte=0;
						break;
					default:
						break;
				}

				next_clock_state=FALL;
				break;    //end of high

			case FALL:
				clockout=0;
				next_clock_state=LOW;
				break;
			case LOW:
				next_clock_state=RISE;
				break;
			default:
				break;
		}


			
		}


	
	switch(mouse_state)
	{
	 
   		case IDLE :
				PORD.bit1=0;
				PORD.bit2=1;
				PORD.bit3=1;
				PORD.bit4=1;
				PORD.bit5=1;
			if((clock_state==RISE || clock_state==HIGH) && clockin==0) // check for stop of txn action from host
			{
				mouse_state=ABORT;
				next_clock_state=LOW;
				txn=0;
				rxn=0;
				
			}
			else
			{
				if(msg_array_empty==0)
				{
					mouse_state=BUSY;
					txn=1;
					rxn=0;	
					next_clock_state=RISE;	
					bit_index=0;	
				}

				else
				{
					mouse_state=IDLE;
					
				}

			}

			break;


		case BUSY:	PORD.bit1=1;						
				PORD.bit2=0;
				PORD.bit3=1;
				PORD.bit4=1;
				PORD.bit5=1;
			if((clock_state==RISE || clock_state==HIGH) && clockin==0) // check for stop of txn action from host
			{
				mouse_state=ABORT;
				next_clock_state=LOW;
				txn=0;
				rxn=0;
			}

			else
			{
				if(!(txn || rxn))				// if not txn or rxn be in idle state
				{
					mouse_state=IDLE;
				}

				else

				{
					mouse_state=BUSY;
								// else be in idle state
				}
			}
 
			
			break;

		case ABORT:
				PORD.bit1=1;						
				PORD.bit2=1;
				PORD.bit3=0;
				PORD.bit4=1;
				PORD.bit5=1;
			if(bit_index>0 && bit_index<=10)                  // reset bit index as its either h->d or d->h

			{			
				resend=1;
			}

				bit_index=0;
				


			if (clockin==1)			// check if host has released the clock??????
			
			
			{				
				mouse_state=REQUEST_WAITING;
				
			}

			
			else    // stay in abort state only

			{
				mouse_state=ABORT;
				next_clock_state=LOW;
				txn=0;
				rxn=0;
			}

			break;

		case REQUEST_WAITING:
				PORD.bit1=1;						
				PORD.bit2=1;
				PORD.bit3=1;
				PORD.bit4=0;
				PORD.bit5=1;


    			if(datain==0)		// check has data line gone low???? i.e, any req from host?

			{
				mouse_state=REQUEST;
				next_clock_state=FALL;
			}
			else		// if dataline is high be in idle state
			{	
				mouse_state=IDLE;
				next_clock_state=RISE;
			}

			break;

		case REQUEST:

				PORD.bit1=1;						
				PORD.bit2=1;
				PORD.bit3=1;
				PORD.bit4=1;
				PORD.bit5=0;
				PORD.bit6=0;
				

			txn=0;
			rxn=1;		// set rxn as the current state of commn
			bit_index=0;
			next_clock_state=RISE;

			if( (clock_state==RISE || clock_state==HIGH) && clockin==0) // check for stop of txn action from host
			{
				mouse_state=ABORT;
				next_clock_state=LOW;
				txn=0;
				rxn=0;
			}
			
			else if(txn||rxn)
				mouse_state=BUSY;
			else
			
				mouse_state=REQUEST;
			break;
		

		default:
			break;
	}// end of switch mouse state



PORD.bit0=1;	

	
}      //end of ISR		
	


 




void get()
{
	int i;

		i=PINB;
		PORTC=i;

			switch(i)
			{

			case 0x01:					//left click		
				msg_array_insert(0x09);
				msg_array_insert(0x00);
				msg_array_insert(0x00);
				break;
			case 0x02:					//right click		
				msg_array_insert(0x0A);
				msg_array_insert(0x00);
				msg_array_insert(0x00);
				break;
			case 0x04:					//move up		
				msg_array_insert(0x08);
				msg_array_insert(0x00);
				msg_array_insert(0x01);
				break;
			case 0x08:					//move down			
				msg_array_insert(0x28);
				msg_array_insert(0x00);
				msg_array_insert(0xFF);
				break;
			case 0x10:					//move right		
				msg_array_insert(0x08);
				msg_array_insert(0x01);
				msg_array_insert(0x00);
				break;
			case 0x20:					//move left
				msg_array_insert(0x18);
				msg_array_insert(0xFF);
				msg_array_insert(0x00);
				break;
			default:					// no action
				msg_array_insert(0x08);
				msg_array_insert(0x00);
				msg_array_insert(0x00);
				break;
			}

	}


void process_cmd(unsigned char x)
{
	
	if(x==RESET)
	{
		msg_array_in_pointer=0;
		msg_array_out_pointer=0;
		msg_array_empty=1;
		msg_array_full=0;
		_delay_us(500);
		PORD.bit3=0;
	}
	
	msg_array_insert(ACK);

	if(wait_for_res)
	{
		if( (x>=0) && (x<4))
		{
			resolution=x;
			
		}
		else
		{
			msg_array_insert(ERROR);		
		}
		wait_for_res=0;
		return;
	}
	
	if(wait_for_SR)
	{
		if( (x>=10) && (x<=200) )		
			sample_rate=x;
		

		else						
		
			msg_array_insert(ERROR);


		wait_for_SR=0;
		return;
	}

	
	switch(x)
	{
		case RESET:
				data_reporting=0;
				sample_rate=0x64;
				scaling=0;
				resolution=0x02;

				msg_array_insert(ACK);
				msg_array_insert(BAT);
				msg_array_insert(MOUSE_ID);	
				
								
			break;
		case RESEND:
			
			resend=1;
			break;
	
		case  ENABLE_DATA_REPORTING:
							//mouse detected
				data_reporting=1;
				msg_array_insert(ACK);
			break;

		case GET_DEVICE_ID:
				msg_array_insert(ACK);
				msg_array_insert(MOUSE_ID);		
			break;

		case SET_SCALING11:
				scaling=0;
				msg_array_insert(ACK);
			break;

		case SET_SCALING21:
				scaling=1;
				msg_array_insert(ACK);
			break;
		case SET_RESOLUTION:
			
				wait_for_res=1;

			break;
		case STATUS_REQUEST:
				msg_array_insert(ACK);
				msg_array_insert(MOUSE_ID);		//sdfasgasfgasfgsdf
				msg_array_insert(resolution);		
				msg_array_insert(sample_rate);
				
			break;
		case SET_SAMPLE_RATE:
					wait_for_SR=1;
					msg_array_insert(ACK);
		
			break;

		case  DISABLE_DATA_REPORTING:

					data_reporting=0;	
					msg_array_insert(ACK);
												
			break;
		case SET_DEFAULT:

				sample_rate=0x64;
				resolution=4;
				data_reporting=0;
				msg_array_insert(ACK);
			break;
		case ERROR:
				msg_array_insert(ACK);
				msg_array_insert(previous_byte);
			break;
		case SET_REMOTE_MODE:
				remote_mode=1;
				msg_array_insert(ACK);
			break;
		case SET_WARP_MODE:
				warp_mode=1;
				msg_array_insert(ACK);
			break;
		case RESET_WARP_MODE:
				warp_mode=0;
				msg_array_insert(ACK);
			break;
		case READ_DATA:
				msg_array_insert(ACK);
				msg_array_insert(B1);
				msg_array_insert(B2);
				msg_array_insert(B3);
			break;
		case SET_STREAM_MODE:
				stream_mode=1;
				msg_array_insert(ACK);
			break;
	
		default:	PORD.bit4=0;
		
			break;
	}


	
}

				

unsigned char get_parity(unsigned char x)
{
	unsigned char temp,i;
	temp=1;
	for(i=0;i<8;i++)
	{

		temp=temp^(x &1);
		x>>=1;
	}

	return temp;
}


unsigned char msg_array_insert(unsigned char x)
{
	if(msg_array_full==1)
		return 1;
	PORTC=x;
	msg_array[msg_array_in_pointer]=x;
	msg_array_in_pointer++;
	msg_array_empty=FALSE;
	
	if (msg_array_in_pointer==msg_array_len)
		msg_array_in_pointer=0;

	if(msg_array_in_pointer==msg_array_out_pointer	)
		msg_array_full=1;

	return 0;
}

unsigned char msg_array_get(void)
{

	char a;
	
	if(resend==1)
	{
		resend=0;
		return (previous_byte)		;
	}
	
	if(msg_array_empty==1)
	
		return 0;
	a=msg_array[msg_array_out_pointer];
	msg_array_out_pointer++;
	msg_array_full=0;
	if(msg_array_out_pointer==msg_array_len)
	{
		msg_array_out_pointer=0;
	}
	
	if(msg_array_out_pointer==msg_array_in_pointer)
	{
		msg_array_empty=1;
	}

previous_byte=a;


return(a);

}


