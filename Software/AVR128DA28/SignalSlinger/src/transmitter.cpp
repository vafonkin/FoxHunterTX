/*
 *  MIT License
 *
 *  Copyright (c) 2022 DigitalConfections
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */


#include <string.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "transmitter.h"
#include "port.h"
#include "binio.h"
// #include "i2c.h"    /* DAC on 80m VGA of Rev X1 Receiver board */
// #include "dac0.h"

static volatile bool g_tx_initialized = false;
volatile Frequency_Hz g_80m_frequency = EEPROM_FREQUENCY_DEFAULT;
volatile uint16_t g_80m_power_level_mW = EEPROM_TX_80M_POWER_MW_DEFAULT;
volatile Frequency_Hz g_rtty_offset = EEPROM_RTTY_OFFSET_FREQUENCY_DEFAULT;

volatile bool g_tx_power_is_zero = true;

static volatile bool g_drain_voltage_enabled = false;
static volatile bool g_final_output_setting = OFF;
static volatile bool g_rf_output_inhibited = false;		

uint16_t g_80m_power_table[16] = DEFAULT_80M_POWER_TABLE;

/*
 * Local Function Prototypes
 */
void final_drain_voltage(bool state);


/*
 *       This function sets the VFO frequency (CLK0 of the Si5351) based on the intended frequency passed in by the parameter (freq),
 *       and the VFO configuration in effect. The VFO  frequency might be above or below the intended  frequency, depending on the VFO
 *       configuration setting in effect for the radio band of the frequency.
 */
	bool txSetFrequency(Frequency_Hz *freq, bool leaveClockOff)
	{
		bool err = true;

		if(!freq) return(err);
		
		if((*freq < TX_MAXIMUM_FREQUENCY) && (*freq > TX_MINIMUM_FREQUENCY))    /* 80m */
		{
			if(!si5351_set_freq(*freq, TX_CLOCK_HF_0, leaveClockOff))
			{
				g_80m_frequency = *freq;
				err = false;
			}
		}

		return(err);
	}

	Frequency_Hz txGetFrequency(void)
	{
		return( g_80m_frequency);
	}

	EC powerToTransmitter(bool state)
	{
		g_final_output_setting = state;
		
		if(g_rf_output_inhibited)
		{
			final_drain_voltage(OFF);
		}
		else
		{
			final_drain_voltage(state);
		}

		return(ERROR_CODE_NO_ERROR);
	}
	
	void txKeyDown(bool key)
	{
		if(g_tx_initialized)
		{
			int tries = 10;
			while(tries-- && (key != keyTransmitter(key)));
		}
	}
	
	void fet_driver(bool state)
	{
		if(state == ON)
		{
			PORTA_set_pin_level(FET_DRIVER_ENABLE, HIGH);
		}
		else
		{
			PORTA_set_pin_level(FET_DRIVER_ENABLE, LOW);
		}
	}

	void final_drain_voltage(bool state)
	{
// 		g_drain_voltage_enabled = state;
// 
// 		if(state == ON)
// 		{
// 			if(!g_rf_output_inhibited)
// 			{
// 				PORTA_set_pin_level(V3V3_PWR_ENABLE, HIGH);
// 			}
// 		}
// 		else
// 		{
// 			PORTA_set_pin_level(V3V3_PWR_ENABLE, LOW);
// 		}
	}

	
	void inhibitRFOutput(bool inhibit)
	{
// 		g_rf_output_inhibited = inhibit;
// 		
// 		if(inhibit)
// 		{
// 			PORTA_set_pin_level(V3V3_PWR_ENABLE, LOW);
// 		}
// 		else
// 		{
// 			uint16_t pwr_mW = g_80m_power_level_mW;
// 			txSetParameters(&pwr_mW, NULL);
// 			PORTA_set_pin_level(V3V3_PWR_ENABLE, g_final_output_setting);
// 		}
	}

	bool keyTransmitter(bool on)
	{
		static volatile bool transmitter_keyed = false;
		
		if(g_tx_initialized)
		{			
			int tries = 5;
			
			if(on)
			{
				if(!transmitter_keyed)
				{
					while(tries-- && (si5351_clock_enable(TX_CLOCK_HF_0, SI5351_CLK_ENABLED) != ERROR_CODE_NO_ERROR))
					{
						shutdown_transmitter();
						restart_transmitter();
					}
					
					if(tries)
					{
						transmitter_keyed = true;
					}
				}
			}
			else
			{
				while(tries-- && (si5351_clock_enable(TX_CLOCK_HF_0, SI5351_CLK_DISABLED) != ERROR_CODE_NO_ERROR))
				{
					shutdown_transmitter();
					restart_transmitter();
				}
					
				if(tries)
				{
					transmitter_keyed = false;
				}
			}
		}
		
		return(transmitter_keyed);
	}

	uint16_t txGetPowerMw(void)
	{
		return( g_80m_power_level_mW);
	}
	
	bool txIsInitialized(void)
	{
		return g_tx_initialized;
	}

	void shutdown_transmitter(void)
	{
		si5351_shutdown_comms();	
	}
	
	void restart_transmitter(void)
	{
		si5351_start_comms();
	}

	EC init_transmitter(Frequency_Hz freq)
	{
		g_80m_frequency = freq;
		return init_transmitter();
	}
	
	EC init_transmitter(void)
	{
		EC code;
		bool err;
				
		fet_driver(OFF);		
//		DAC0_init();

		if((err = si5351_init(SI5351_CRYSTAL_LOAD_6PF, 0)))
		{
			return(ERROR_CODE_RF_OSCILLATOR_ERROR);
		}

		if((code = si5351_drive_strength(TX_CLOCK_HF_0, SI5351_DRIVE_2MA)))
		{
			return( code);
		}
		
		if((code = si5351_clock_enable(TX_CLOCK_HF_0, SI5351_CLK_DISABLED)))
		{
			return( code);
		}

		err = txSetFrequency((Frequency_Hz*)&g_80m_frequency, true);
		if(!err)
		{
			g_tx_initialized = true;
		}
		
		fet_driver(ON);

		return( code);
	}


EC txMilliwattsToSettings(uint16_t* powerMW, uint16_t* driveLevel)
{
	EC ec = ERROR_CODE_NO_ERROR;
	uint16_t maxPwr;
	uint8_t index;

	if(powerMW == NULL)
	{
		return(ERROR_CODE_SW_LOGIC_ERROR);
	}

	maxPwr = MAX_TX_POWER_80M_MW;

	if(*powerMW > maxPwr)
	{
		ec = ERROR_CODE_POWER_LEVEL_NOT_SUPPORTED;
	}

	*powerMW = CLAMP((uint16_t)0, *powerMW, maxPwr);

	if(*powerMW < 5)
	{
		index = 0;
		*powerMW = 0;
	}
	else if(*powerMW < 50)
	{
		index = 1;
		*powerMW = 10;
	}
	else if(*powerMW < 150)
	{
		index = 2;
		*powerMW = 100;
	}
	else if(*powerMW < 250)
	{
		index = 3;
		*powerMW = 200;
	}
	else if(*powerMW < 350)
	{
		index = 4;
		*powerMW = 300;
	}
	else if(*powerMW < 450)
	{
		index = 5;
		*powerMW = 400;
	}
	else if(*powerMW < 550)
	{
		index = 6;
		*powerMW = 500;
	}
	else if(*powerMW < 650)
	{
		index = 7;
		*powerMW = 600;
	}
	else if(*powerMW < 900)
	{
		index = 8;
		*powerMW = 800;
	}
	else if(*powerMW < 1250)
	{
		index = 9;
		*powerMW = 1000;
	}
	else if(*powerMW < 1750)
	{
		index = 10;
		*powerMW = 1500;
	}
	else if(*powerMW < 2250)
	{
		index = 11;
		*powerMW = 2000;
	}
	else if(*powerMW < 2750)
	{
		index = 12;
		*powerMW = 2500;
	}
	else if(*powerMW < 3500)
	{
		index = 13;
		*powerMW = 3000;
	}
	else if(*powerMW < 4500)
	{
		index = 14;
		*powerMW = 4000;
	}
	else
	{
		index = 15;
		*powerMW = 5000;
	}

	*driveLevel = g_80m_power_table[index];
	*driveLevel = MIN(*driveLevel, MAX_80M_PWR_SETTING);

	return(ec);
}