#include "framework.h"

/* 
 * Sets up the hardware like pins, timer and interrupts
 * that is required to receive IR signals
 */
void ir_setup();

/* 
 * Should be called from the interrupt service method.
 * Checks if the interrupt is caused by either the data pin 
 * where the IR receiver is attached or by the timer.
 */
void ir_interruptHandler();

/*
 * Tells if a command has been received.
 * Returns true, if command is in the receive-buffer.
 * You can call ir_popLatestCommand afterwards.
 * Can be polled from the main method.
 */
bool ir_isReady();

/* 
 * Returns the latest received command.
 * Resets the flag that a command is valid, so that
 * a new command can be received.
 */
uint8 ir_popLatestCommand();
