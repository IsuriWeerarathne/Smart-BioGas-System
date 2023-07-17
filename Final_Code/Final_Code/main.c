#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "LCD_16x2_H_file.h"


void ADC_Init(){
	DDRA &= ~(1<<PA0);								/* Make ADC port pin 0 as input */
	ADCSRA = 0x87;									/* Enable ADC, with freq/128  */
	ADMUX = 0x40;									/* Vref: Avcc, ADC channel: 0 */
}

int ADC_Read(char channel)
{
	ADMUX = 0x40 | (channel & 0x07);				/* set input channel to read */
	ADCSRA |= (1<<ADSC);							/* Start ADC conversion */
	while (!(ADCSRA & (1<<ADIF)));					/* Wait until end of conversion by polling ADC interrupt flag */
	ADCSRA |= (1<<ADIF);							/* Clear interrupt flag */
	_delay_ms(1);									/* Wait a little bit */
	return ADCW;									/* Return ADC word */
}


void ADC_Init1(){
	DDRA &= ~(1<<PA1); 							    /* Make ADC port pin 1 as input */
	ADCSRA = 0x87;									/* Enable ADC, with freq/128  */
	/* Vref: Avcc, ADC channel: 0 */
}

int ADC_Read1()
{
	ADMUX = 0x41;									/* set input channel to read */
	ADCSRA |= (1<<ADSC);							/* Start ADC conversion */
	while (!(ADCSRA & (1<<ADIF)));					/* Wait until end of conversion by polling ADC interrupt flag */
	ADCSRA |= (1<<ADIF);							/* Clear interrupt flag */
	_delay_ms(1);									/* Wait a little bit */
	return ADCW;									/* Return ADC word */
}

//define values to calculate PPM (line 51 to 54) curve points

float b_of_lpg=1.25;
float m_of_lpg=-0.45;
float b_of_Methane=1.34;
float m_of_Methane=-0.37;
float Mq2_Anlog_value = 0;

//level view code
void view_level(){
	LCD_Clear();
	_delay_ms(50);
	LCD_String_xy(1,0,"  liquid level ");		//Display if liquid level is high
	LCD_String_xy(2,0,"    is high");
}


int main()

{
	//Following 3 pins for button level sensor
	DDRD &= ~(1<<PD2);								//Make PD2 port as input pin for level sensor
	DDRC |= (1<<PC5);								// Make PC5 port as output pin for water solenoid valve
	DDRD &= ~(1<<PD5);								//Make PD5 port as input pin for the button to remove liquid
	
	DDRD =DDRD | (1<<4);							// turn off the solenoid valve--- Lp gas  make as output port
	DDRC = DDRC | (1<<3);							//turn off bio gas supply --- make as output port
	DDRC = DDRC | (1<<4);							//make port c solenoid valve as output to the outside
	
	DDRD &=~(1<<PD6);								// button to demontrate gas sensor
	DDRD = DDRD | (1<<7);							//make port d pin 7 as output to buzzer
	
	
	char pressure[10];
	float kiloPaskal;

	LCD_Init();										/* initialize 16x2 LCD*/
	ADC_Init();										/* initialize ADC*/
	
	
	while(1)
	{
			LCD_Clear();
		    LCD_String_xy(1,2,"SMART BIO GAS");
		    LCD_String_xy(2,5,"SYSTEM");
		    _delay_ms(300);
		    LCD_Clear();

		//Code lines for buton to remove liquid
		
		if (PIND & (1<<PD5))
		{
			_delay_ms(50);
			PORTC |= (1<<PC5);
			}else{
			_delay_ms(50);
			PORTC &= ~(1<<PC5);
		}
		
		
		kiloPaskal = (ADC_Read(0)*4.88);
		kiloPaskal = ((((kiloPaskal+0.3)/255)-0.04)/0.009);		//calculate pressure
		_delay_ms(1000);
		memset(pressure,0,10);
		
		
		if (kiloPaskal>=1581)									//When bio gas pressure goes high
		{
			if (PIND & (1<<PD2))								//to display level out and gas in use
			{
				view_level();
				}else{
				LCD_Clear();
				_delay_ms(50);
				LCD_String_xy(1,0,"    Biogas go");
				LCD_String_xy(2,0,"     outside");
			}
			
			PORTC |=(1<<PC3);									//Turn on bia gas supply
			PORTC |=(1<<PC4);									//turn on bio gas output(to outside) solenoid valve
			PORTD &=~(1<<PD4);									//Turn off LP gas supply
			
		}else if (kiloPaskal>=400)
		{
			if (PIND & (1<<PD2))
			{
				view_level();
				}else{
				LCD_Clear();
				_delay_ms(50);
				LCD_String_xy(1,0,"     Biogas");
				LCD_String_xy(2,0,"     in use");
			}
			
			PORTC &=~(1<<PC4);									//turn off bio output sol
			PORTC |=(1<<PC3);									//Turn on bia gas supplyenoid valve
			PORTD &=~(1<<PD4);									//Turn off LP gas supply
			}else{
			if (PIND & (1<<PD2))
			{
				view_level();
				}else{
				LCD_Clear();
				_delay_ms(50);
				LCD_String_xy(1,0,"      LPgas");
				LCD_String_xy(2,0,"     in use");
			}
			
			PORTC &=~(1<<PC3);								//Turn off bio gas supply
			PORTD |=(1<<PD4);								//Turn on Lp gas supply
			PORTC &=~(1<<PC4);								//turn off bio output solenoid valve
		}
		


		//close all the solenoid valves --- to show the work of gas sensor
		if (PINA & (1<<PA1))
		{
			LCD_Clear();
			PORTC &=~(1<<PC3);									//Turn off bio gas supply
			PORTC &=~(1<<PC4);									//turn off bio output solenoid valve
			PORTD &=~(1<<PD4);									//Turn off LP gas supply
			PORTD |= (1<<PD7);									//Turn on the buzzer
			
			LCD_String_xy(1,0,"*** Gas leak ***");
			_delay_ms(100);
			}else{
			PORTD &= ~(1<<PD7);									//Turn off the buzzer
		}
		
		
		//	gas sensor up to line 224 // commented beacues  gas sensor is not working in protues
		
				ADC_Init1();
		 		_delay_ms(1000);
		 		float ADC_input_mq2 = ADC_Read(1);							//get the ADC pin 1 input
		
		 		for (int x=0;x<20;x++)
		 		{
					float Mq2_Anlog_value = ADC_input_mq2/20;				//take analog average readings from the sensor until stable
		 			return Mq2_Anlog_value;
		 		}
		 		Mq2_Anlog_value = Mq2_Anlog_value/20;
		 		float Mq2_V_Volts = (Mq2_Anlog_value *5)/1023;				//161 line & this line to convert analog input to voltage
		 		float RS_of_Air = (5-Mq2_V_Volts)/Mq2_V_Volts;				//Take RS of the air
		 		float RO = RS_of_Air/9.8;
		 		_delay_ms(50);
		
		 		float RS_RO_Ratio = RS_of_Air/ RO;
		 		float PPM_Log_lpg = (log(RS_RO_Ratio)-b_of_lpg)/m_of_lpg;	//getting PPM value in log for LPG
		 		float PPM_Log_methane = (log(RS_RO_Ratio)-b_of_Methane)/m_of_Methane; //getting PPM value in log for methane
		
		 		float PPM_L =  pow(10,PPM_Log_lpg);							//get the inverse of the log value to find the real values
		 		float PPM_M =  pow(10,PPM_Log_methane);						//get the inverse of the log value to find the real values
		
		//Wrote condition based on assumptions
		
		 		if (PPM_L > 2000) //assume if lpg goes higher than 2000 it is danger
		 		{
		 			PORTC &=~(1<<PC3);										//Turn off bio gas supply
		 			PORTC &=~(1<<PC4);										//turn off bio output solenoid valve
		 			PORTD &=~(1<<PD4);										//Turn off LP gas supply
		 			PORTD |= (1<<PD7);										//Turn on the buzzer
		 			LCD_Clear();
		 			LCD_String_xy(1,0,"Gas leak -LPG");
		 			_delay_ms(100);
		
		 		}else if (PPM_M > 1000)										//assume if Methane goes higher than 1000 it is danger
		 		{
		 			PORTC &=~(1<<PC3);										//Turn off bio gas supply
		 			PORTC &=~(1<<PC4);										//turn off bio output solenoid valve
		 			PORTD &=~(1<<PD4);										//Turn off LP gas supply
					PORTD |= (1<<PD7);										//Turn on the buzzer
		 			LCD_Clear();
		 			LCD_String_xy(1,0,"Gas leak -Methane");
		 			_delay_ms(100);
		
		 		}else{
		 			PORTD &= ~(1<<PD7);										//Turn off the buzzer
		 		}
		

		
		
	}
	


	
	return 0;
}


