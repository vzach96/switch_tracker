#include <stdio.h>
#include <pigpio.h>
#include "common.h"

int initGpio()
{
	int cfg = gpioCfgGetInternals();
	cfg |= PI_CFG_NOSIGHANDLER;
	gpioCfgSetInternals(cfg);
	int status = gpioInitialise();
	// Set mode of gpio pins
	// set select pins as output
	status |= gpioSetMode(17, PI_OUTPUT);	//S5
	status |= gpioSetMode(27, PI_OUTPUT);	//S4
	status |= gpioSetMode(22, PI_OUTPUT);	//S3
	status |= gpioSetMode(5, PI_OUTPUT);	//S2
	status |= gpioSetMode(6, PI_OUTPUT);	//S1
	status |= gpioSetMode(26, PI_OUTPUT);	//S0
	// set output pins as input
	status |= gpioSetMode(23, PI_INPUT);	//Y
	status |= gpioSetMode(24, PI_INPUT);	//W=~Y

	return status;
}

int readInputPins()
{
	int Y = gpioRead(23);
	int W = gpioRead(24);

	//return Y & ~W;
	return Y;
}

int setCountToOutputPins(unsigned count)
{
	unsigned select[6] = {0,0,0,0,0,0};
	select[0] = count & 1;
	select[1] = (count >> 1) & 1;
	select[2] = (count >> 2) & 1;
	select[3] = (count >> 3) & 1;
	select[4] = (count >> 4) & 1;
	select[5] = (count >> 5) & 1;

	int retc = 0;
	retc |= gpioWrite(26, select[0]);
	retc |= gpioWrite(6, select[1]);
	retc |= gpioWrite(5, select[2]);
	retc |= gpioWrite(22, select[3]);
	retc |= gpioWrite(27, select[4]);
	retc |= gpioWrite(17, select[5]);

	return retc;
}

void terminate()
{
	gpioTerminate();
}
