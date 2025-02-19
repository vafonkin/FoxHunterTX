/*
 *  MIT License
 *
 *  Copyright (c) 2021 DigitalConfections
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
/*
 * serialbus.h - a simple serial inter-processor communication protocol.
 */

#ifndef SERIALBUS_H_
#define SERIALBUS_H_

#include "defs.h"
#include "usart_basic.h"

#ifdef __cplusplus
	extern "C" {
#endif

#define SERIALBUS_USART USART_1

#define SERIALBUS_MAX_MSG_LENGTH 50
#define SERIALBUS_MIN_MSG_LENGTH 2    /* shortest message: GO */
#define SERIALBUS_MAX_MSG_FIELD_LENGTH 20
#define SERIALBUS_MAX_MSG_NUMBER_OF_FIELDS 3
#define SERIALBUS_NUMBER_OF_RX_MSG_BUFFERS 2
#define SERIALBUS_MAX_TX_MSG_LENGTH 41
#define SERIALBUS_NUMBER_OF_TX_MSG_BUFFERS 3
#define SERIALBUS_MAX_MSG_ID_LENGTH 3

#define SERIALBUS_MAX_COMMANDLINE_LENGTH ((1 + SERIALBUS_MAX_MSG_FIELD_LENGTH) * SERIALBUS_MAX_MSG_NUMBER_OF_FIELDS)

#define SERIALBUS_POWERUP_DELAY_SECONDS 6

#define SERIALBUS_MIN_TX_INTERVAL_MS 100

#define SB_BAUD 9600

//#define MYUBRR(b) ((F_CPU + b * 8L) / (b * 16L) - 1)

// typedef enum
// {
// 	SB_EMPTY_BUFF,
// 	SB_FULL_BUFF
// } BufferState;

/*  Serialbus Messages
 *       Message formats:
 *               CMD [a1 [a2]]
 *
 *               where
 *                       CMD = command
 *                       a1, a2 = optional arguments or data fields
 *
 */

typedef enum
{
	SB_MESSAGE_EMPTY = 0,

	/*	ARDUCON MESSAGE FAMILY (SERIAL MESSAGING) */
	SB_MESSAGE_SET_FOX = 'F' * 100 + 'O' * 10 + 'X',			/* Set the fox role to be used to define timing and signals */
	SB_MESSAGE_VOLTS = 'B' * 100 + 'A' * 10 + 'T',				/* Battery voltage and threshold setting */
	SB_MESSAGE_SET_STATION_ID = 'I' * 10 + 'D',					/* Sets amateur radio callsign text */
	SB_MESSAGE_GO = 'G' * 10 + 'O',								/* Start/stop transmissions */
	SB_MESSAGE_CODE_SETTINGS = 'S' * 100 + 'P' * 10 + 'D',		/* Set Morse code speeds */
	SB_MESSAGE_CLOCK = 'C' * 100 + 'L' * 10 + 'K',				/* Set or read the RTC */
	SB_MESSAGE_MASTER = 'M' * 100 + 'A' * 10 + 'S',				/* Set master role command */
	SB_MESSAGE_EVENT = 'E' * 100 + 'V' * 10 + 'T',				/* Set event */
	SB_MESSAGE_TX_FREQ = 'F' * 100 + 'R' * 10 + 'E',			/* Transmit frequency */
	SB_MESSAGE_PATTERN = 'P' * 100 + 'A' *10 + 'T',				/* Set the transmit pattern */    
	SB_MESSAGE_KEY = 'K' * 100 + 'E' * 10 + 'Y',				/* Key on/off */
	SB_MESSAGE_SLP = 'S' * 100 + 'L' * 10 + 'P',				/* Sleep */
	SB_MESSAGE_VER = 'V' * 100 + 'E' * 10 + 'R',				/* Version */
	SB_MESSAGE_HELP = '?',										/* Help */
	SB_MESSAGE_DEBUG = 'D' * 100 + 'B' * 10 + 'G',				/* Debug information */
	SB_MESSAGE_FUNCTION = 'F' * 100 + 'U' * 10 + 'N',			/* Functionality setting */

	SB_INVALID_MESSAGE = MAX_UINT16								/* This value must never overlap a valid message ID */
} SBMessageID;

typedef enum
{
	SERIALBUS_MSG_UNKNOWN = 0,
	SERIALBUS_MSG_COMMAND,
	SERIALBUS_MSG_QUERY,
	SERIALBUS_MSG_REPLY,
	SERIALBUS_MSG_INVALID
} SBMessageType;

typedef enum
{
	SB_FIELD1 = 0,
	SB_FIELD2 = 1
} SBMessageField;

typedef enum
{
	SB_BATTERY_BROADCAST = 0x0001,
	SB_RSSI_BROADCAST = 0x0002,
	SB_RF_BROADCAST = 0x0004,
	SB_UPC_TEMP_BROADCAST = 0x0008,
	SB_ALL_BROADCASTS = 0x000FF
} SBbroadcastType;

// typedef enum
// {
// 	SB_NO_ID = 0,
// 	SB_CONTROL_HEAD_ID = 1,
// 	SB_RECEIVER_ID = 2,
// 	SB_TRANSMITTER_ID = 3
// } DeviceID;

typedef char SerialbusTxBuffer[SERIALBUS_MAX_TX_MSG_LENGTH];

typedef struct
{
	SBMessageType type;
	SBMessageID id;
	char fields[SERIALBUS_MAX_MSG_NUMBER_OF_FIELDS][SERIALBUS_MAX_MSG_FIELD_LENGTH];
} SerialbusRxBuffer;

#define WAITING_FOR_UPDATE -1
#define HELP_TEXT_TXT (char*)"\n* Commands:\n* > CLK [T|S|F|D [\"YYMMDDhhmmss\"]] - Read/set time/start/finish/days\n* > EVT [B|C|F|S] - Set event\n* > FOX [fox]- Set fox role\n* > FRE [frequency] - Set tx frequency\n* > FUN [A|Q|S] - Set device functionality\n* > ID [callsign] -  Set callsign\n* > KEY [1|0] - key down/up\n* > MAS 1 - Set master\n* > PAT [text] - Set xmit pattern\n* > SPD S|P [wpm] - Set ID code speed\n* > GO 0-3 - Start event\n* > BAT [v] - Battery volts\n\0"


/**
 */
void serialbus_init(uint32_t baud, USART_Number_t usart);

/**
 * Immediately turns off receiver and flushes receive buffer
 */
void serialbus_disable(void);

/**
 */
void serialbus_end_tx(void);

/**
 */
// void serialbus_reset_rx(void);

/**
 */
SerialbusTxBuffer* nextEmptySBTxBuffer(void);

/**
 */
SerialbusTxBuffer* nextFullSBTxBuffer(void);

/**
 */
bool serialbusTxInProgress(void);

/**
 */
SerialbusRxBuffer* nextEmptySBRxBuffer(void);

/**
 */
SerialbusRxBuffer* nextFullSBRxBuffer(void);

/**
 */
void sb_send_NewPrompt(void);

/**
 */
void sb_send_NewLine(void);

/**
 */
void sb_echo_char(uint8_t c);

/**
 */
bool sb_send_string(char* str);
bool sb_send_master_string(char* str);

/**
 */
void sb_send_value(uint16_t value, char* label);

/**
 */
bool sb_enabled(void);

#ifdef __cplusplus
	}
#endif

#endif  /* SERIALBUS_H_ */
