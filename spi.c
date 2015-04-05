#include "spi.h"


uint spi_write_2bytes(uint _data) {
    uint ret1;
    uint ret2;
    uchar byte1 = ((_data & 0xFF00) >> 8);
    uchar byte2 = _data & 0x00FF;

    nSPI_SLAVE_SELECT=0;       //activate slave
    delayms(1);

    SSPBUF=byte1;             //send byte1
    while (!SSPSTATbits.BF);  //wait for transmission complete
    ret1=SSPBUF;              //read answer from buffer

    SSPBUF=byte2;             //send byte2
    while (!SSPSTATbits.BF);  //wait for transmission complete
    ret2=SSPBUF;

    delayms(1);
    nSPI_SLAVE_SELECT=1;       //deactivate slave

    return (ret1 << 8) | ret2;
}
