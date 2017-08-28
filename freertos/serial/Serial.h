/* 
 * File:   Serial.h
 * Author: martin
 *
 * Created on December 6, 2013, 10:29 PM
 */

#ifndef HARDWARESERIAL_H
#define	HARDWARESERIAL_H


#include <xc.h>

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
bool Serial_begin(UART_MODULE uart, unsigned long baudRate,
        UART_LINE_CONTROL_MODE lineControl, int _rxBufferSize, int _txBufferSize);
        
void Serial_close(UART_MODULE UART);

int Serial_available(UART_MODULE UART);             //>0 if data available

UART_DATA Serial_peek(UART_MODULE UART);          //read a byte or word
UART_DATA Serial_read(UART_MODULE UART);          //read a byte or word
UART_DATA Serial_read_wait(UART_MODULE uart, int ticksToWait); //read with a max wait

void Serial_watch_for_address(UART_MODULE UART, unsigned int address);

void Serial_write(UART_MODULE UART, const char  dat);       //write a byte
void Serial_writeData(UART_MODULE UART, const UART_DATA dat);       //write 8/9 bits

void Serial_write_bytes(UART_MODULE UART, const char *dat, int len);
void Serial_write_string(UART_MODULE UART, const char *str);

void Serial_purge(UART_MODULE uart);    //discard read data
void Serial_flush(UART_MODULE UART);    //complete transmission

//Interrupt handlers
void UART_1A_ISR_Handler();
void UART_2A_ISR_Handler();
void UART_3A_ISR_Handler();
void UART_1B_ISR_Handler();
void UART_2B_ISR_Handler();
void UART_3B_ISR_Handler();

#ifdef	__cplusplus
}
#endif

#endif	/* HARDWARESERIAL_H */

