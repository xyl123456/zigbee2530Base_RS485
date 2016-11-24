//////////////////////////////////////////////////////////////////////////////
//
//                      Copyright (c) 2006 Gerhard H. Schalk
//         
//                  ELEKTOR RFID READER for MIFARE and ISO14443-A
//                 
//////////////////////////////////////////////////////////////////////////////
//   Filename:
//   Comment:
//      
////////////////////////////////////////////////////////////////////////////// 

#ifndef __RC522REG_H__
#define __RC522REG_H__


/*! \name Register definitions of Page 0
 *  \ingroup reg
 *  Following all register defintions of the RC522 Page 0.
 */
/*@{*/
#define     RC522_REG_RFU00           0x00      /*   Currently not used.                                     */
#define     RC522_REG_COMMAND         0x01      /*!< Contains Command bits, PowerDown bit and bit to
                                                switch receiver off.                                    */
#define     RC522_REG_COMMIEN         0x02      /*!< Contains Communication interrupt enable bits and
                                                bit for Interrupt inversion.                            */
#define     RC522_REG_DIVIEN          0x03      /*!< Contains RfOn, RfOff, CRC and Mode Interrupt enable
                                                and bit to switch Interrupt pin to PushPull mode.       */
#define     RC522_REG_COMMIRQ         0x04      /*!< Contains Communication interrupt request bits.          */
#define     RC522_REG_DIVIRQ          0x05      /*!< Contains RfOn, RfOff, CRC and Mode Interrupt request.  */
#define     RC522_REG_ERROR           0x06      /*!< Contains Protocol, Parity, CRC, Collision, Buffer
                                                overflow, Temperature and RF error flags.               */
#define     RC522_REG_STATUS1         0x07      /*!< Contains status information about Lo- and HiAlert,
                                                RF-field on, Timer, Interrupt request and CRC status.   */
#define     RC522_REG_STATUS2         0x08      /*!< Contains information about internal states (Modemstate),
                                                Mifare states and possibility to switch Temperature
                                                sensor off.                                             */
#define     RC522_REG_FIFODATA        0x09      /*!< Gives access to FIFO. Writing to register increments the
                                                FIFO level (register 0x0A), reading decrements it.      */
#define     RC522_REG_FIFOLEVEL       0x0A      /*!< Contains the actual level of the FIFO.                  */
#define     RC522_REG_WATERLEVEL      0x0B      /*!< Contains the Waterlevel value for the FIFO              */
#define     RC522_REG_CONTROL         0x0C      /*!< Contains information about last received bits and to
                                                Start and stop the Timer unit.                          */
#define     RC522_REG_BITFRAMING      0x0D      /*!< Contains information of last bits to send, to align
                                                received bits in FIFO and activate sending in Transceive*/
#define     RC522_REG_COLL            0x0E      /*!< Contains all necessary bits for Collission handling     */
#define     RC522_REG_RFU0F           0x0F      /*   Currently not used.                                     */
/*@}*/

/*! \name Register definitions of Page 1
 *  \ingroup reg
 *  Following all register defintions of the RC522 Page 1.
 */
/*@{*/
#define     RC522_REG_RFU10           0x10      /*   Currently not used.                                     */
#define     RC522_REG_MODE            0x11      /*!< Contains bits for auto wait on Rf, to detect SYNC byte in
                                                NFC mode and MSB first for CRC calculation              */
#define     RC522_REG_TXMODE          0x12      /*!< Contains Transmit Framing, Speed, CRC enable, bit for
                                                inverse mode and TXMix bit.                             */
#define     RC522_REG_RXMODE          0x13      /*!< Contains Transmit Framing, Speed, CRC enable, bit for
                                                multiple receive and to filter errors.                  */
#define     RC522_REG_TXCONTROL       0x14      /*!< Contains bits to activate and configure Tx1 and Tx2 and
                                                bit to activate 100% modulation.                        */
#define     RC522_REG_TXASK           0x15      /* */
#define     RC522_REG_TXSEL           0x16      /*!< Contains SigoutSel, DriverSel and LoadModSel bits.      */
#define     RC522_REG_RXSEL           0x17      /*!< Contains UartSel and RxWait bits.                       */
#define     RC522_REG_RXTRESHOLD      0x18      /*!< Contains MinLevel and CollLevel for detection.          */
#define     RC522_REG_DEMOD           0x19      /*!< Contains bits for time constants, hysteresis and
                                                IQ demodulator settings.                                */
#define     RC522_REG_RFU1A           0x1A      /* */
#define     RC522_REG_RFU1B           0x1B      /* */
#define     RC522_REG_RFU1C           0x1C      /* */
#define     RC522_REG_RFU1D           0x1D      /* */
#define     RC522_REG_RFU1E           0x1E      /*   Currently not used.                                     */
#define     RC522_REG_SERIALSPEED     0x1F      /*!< Contains speed settings for serila interface.           */
/*@}*/

/*! \name Register definitions of Page 2
 *  \ingroup reg
 *  Following all register defintions of the RC522 Page 2.
 */
/*@{*/
#define     RC522_REG_RFU20           0x20      /*   Currently not used.                                     */
#define     RC522_REG_CRCRESULT1      0x21	   /*!< Contains MSByte of CRC Result.                          */
#define     RC522_REG_CRCRESULT2      0x22	   /*!< Contains LSByte of CRC Result.                          */
#define     RC522_REG_RFU23           0x23      /*   Currently not used.                                     */
#define     RC522_REG_MODWIDTH        0x24      /*!< Contains modulation width setting.                      */
#define     RC522_REG_RFU25           0x25      /*   Currently not used.                                     */
#define     RC522_REG_RFCFG           0x26      /*!< Contains sensitivity of Rf Level detector, the receiver
                                                gain factor and the RfLevelAmp.                         */
#define     RC522_REG_GSN             0x27      /*!< Contains the conductance and the modulation settings for
                                                the N-MOS transistor. */
#define     RC522_REG_CWGSP           0x28      /*!< Contains the conductance for the P-Mos transistor.      */
#define     RC522_REG_MODGSP          0x29      /*!< Contains the modulation index for the PMos transistor.  */
#define     RC522_REG_TMODE           0x2A      /*!< Contains all settings for the timer and the highest 4
                                                bits of the prescaler.                                  */
#define     RC522_REG_TPRESCALER      0x2B      /*!< Contais the lowest byte of the prescaler.               */
#define     RC522_REG_TRELOADHI       0x2C      /*!< Contains the high byte of the reload value.             */
#define     RC522_REG_TRELOADLO       0x2D      /*!< Contains the low byte of the reload value.              */
#define     RC522_REG_TCOUNTERVALHI   0x2E      /*!< Contains the high byte of the counter value.            */
#define     RC522_REG_TCOUNTERVALLO   0x2F      /*!< Contains the low byte of the counter value.             */
/*@}*/

/*! \name Register definitions of Page 3
 *  \ingroup reg
 *  Following all register defintions of the RC522 Page 3.
 */
/*@{*/
#define     RC522_REG_RFU30           0x30      /*   Currently not used.                                     */
#define     RC522_REG_TESTSEL1        0x31      /*   Test register                                           */
#define     RC522_REG_TESTSEL2        0x32      /*   Test register                                           */
#define     RC522_REG_TESTPINEN       0x33      /*   Test register                                           */
#define     RC522_REG_TESTPINVALUE    0x34      /*   Test register                                           */
#define     RC522_REG_TESTBUS         0x35      /*   Test register                                           */
#define     RC522_REG_AUTOTEST        0x36      /*   Test register                                           */
#define     RC522_REG_VERSION         0x37      /*!< Contains the product number and the version.            */
#define     RC522_REG_ANALOGTEST      0x38      /*   Test register                                           */
#define     RC522_REG_TESTDAC1        0x39      /*   Test register                                           */
#define     RC522_REG_TESTDAC2        0x3A      /*   Test register                                           */
#define     RC522_REG_TESTADC         0x3B      /*   Test register                                           */
#define     RC522_REG_ANALOGUETEST1   0x3C      /*   Test register                                           */
#define     RC522_REG_RFT3D           0x3D      /*   Test register                                           */
#define     RC522_REG_RFT3E           0x3E      /*   Test register                                           */
#define     RC522_REG_RFT3F           0x3F      /*   Test register                                           */
/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Possible commands
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name RC522 Command definitions
 *  \ingroup reg
 *  Following all commands of the RC522.
 */
/*@{*/
#define     RC522_CMD_IDLE          0x00 /*!< No action: cancel current command
                                     or home state. \n */
#define     RC522_CMD_CALCCRC       0x03 /*!< Activate the CRC-Coprocessor \n<em><strong>
                                     Remark: </strong>The result of the CRC calculation can
                                     be read from the register CRCResultXXX </em>*/
#define     RC522_CMD_TRANSMIT      0x04 /*!< Transmit data from FIFO to the card \n<em>
                                     <strong>Remark: </strong>If data is already in
                                     the FIFO when the command is activated, this data is
                                     transmitted immediately. It is possible to
                                     write data to the FIFO while the Transmit
                                     command is active. Thus it is possible to
                                     transmit an unlimited number of bytes in one
                                     stream by writting them to the FIFO in time.</em>*/
#define     RC522_CMD_NOCMDCHANGE   0x07 /*!< This command does not change the actual commant of
                                     the RC522 and can only be written. \n<em><strong>
                                     Remark: </strong>This command is used for WakeUp procedure
                                     of RC522 to not change the current state. </em>*/
#define     RC522_CMD_RECEIVE       0x08 /*!< Activate Receiver Circuitry. Before the
                                     receiver actually starts, the state machine
                                     waits until the time configured in the
                                     register RxWait has passed. \n<em><strong>
                                     Remark: </strong>It is possible to read any received
                                     data from the FIFO while the Receive command
                                     is active. Thus it is possible to receive an
                                     unlimited number of bytes by reading them
                                     from the FIFO in time. </em>*/
#define     RC522_CMD_TRANSCEIVE    0x0C /*!< This Command has two modes:\n
                                     Transmits data from FIFO to the card and after
                                     that automatically activates
                                     the receiver. Before the receiver actually
                                     starts,the state machine waits until the
                                     time configured in the register RxWait has
                                     passed. \n <em><strong>
                                     Remark: </strong>This command is the combination of
                                     Transmit and Receive.</em> */
#define     RC522_CMD_AUTHENT       0x0E /*!< Perform the card authentication using the
                                     Crypto1 algorithm.
                                     \n <em><strong>Remark: </strong></em>*/
#define     RC522_CMD_SOFTRESET     0x0F /*!< Runs the Reset- and Initialisation Phase
                                     \n <em><strong>Remark:</strong> This command can
                                     be activated by software, but only by a Power-On
                                     or Hard Reset </em>*/
/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Bit Definitions
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name RC522 Bit definitions of Page 0
 *  \ingroup reg
 *  Below there are useful bit definition of the RC522 register set of Page 0.
 */
/*@{*/
/* Command Register							(01) */
#define     RC522_BIT_RCVOFF             0x20   /*!< Switches the receiver on/off. */
#define     RC522_BIT_POWERDOWN          0x10   /*!< Switches RC522 to Power Down mode. */

/* CommIEn Register							(02) */
#define     RC522_BIT_IRQINV             0x80   /*!< Inverts the output of IRQ Pin. */

/* DivIEn Register							(03) */
#define     RC522_BIT_IRQPUSHPULL        0x80   /*!< Sets the IRQ pin to Push Pull mode. */

/* CommIEn and CommIrq Register         (02, 04) */
#define     RC522_BIT_TXI                0x40   /*!< Bit position for Transmit Interrupt Enable/Request. */
#define     RC522_BIT_RXI                0x20   /*!< Bit position for Receive Interrupt Enable/Request. */
#define     RC522_BIT_IDLEI              0x10   /*!< Bit position for Idle Interrupt Enable/Request. */
#define     RC522_BIT_HIALERTI           0x08   /*!< Bit position for HiAlert Interrupt Enable/Request. */
#define     RC522_BIT_LOALERTI           0x04   /*!< Bit position for LoAlert Interrupt Enable/Request. */
#define     RC522_BIT_ERRI               0x02   /*!< Bit position for Error Interrupt Enable/Request. */
#define     RC522_BIT_TIMERI             0x01   /*!< Bit position for Timer Interrupt Enable/Request. */

/* DivIEn and DivIrq Register           (03, 05) */
#define     RC522_BIT_SIGINACTI          0x10   /*!< Bit position for SiginAct Interrupt Enable/Request. */
#define     RC522_BIT_CRCI               0x04   /*!< Bit position for CRC Interrupt Enable/Request. */

/* CommIrq and DivIrq Register          (04, 05) */
#define     RC522_BIT_SET                0x80   /*!< Bit position to set/clear dedicated IRQ bits. */

/* Error Register 							(06) */
#define     RC522_BIT_WRERR              0x80   /*!< Bit position for Write Access Error. */
#define     RC522_BIT_TEMPERR            0x40   /*!< Bit position for Temerature Error. */
#define     RC522_BIT_BUFFEROVFL         0x10   /*!< Bit position for Buffer Overflow Error. */
#define     RC522_BIT_COLLERR            0x08   /*!< Bit position for Collision Error. */
#define     RC522_BIT_CRCERR             0x04   /*!< Bit position for CRC Error. */
#define     RC522_BIT_PARITYERR          0x02   /*!< Bit position for Parity Error. */
#define     RC522_BIT_PROTERR            0x01   /*!< Bit position for Protocol Error. */

/* Status 1 Register 						(07) */
#define     RC522_BIT_CRCOK              0x40   /*!< Bit position for status CRC OK. */
#define     RC522_BIT_CRCREADY           0x20   /*!< Bit position for status CRC Ready. */
#define     RC522_BIT_IRQ                0x10   /*!< Bit position for status IRQ is active. */
#define     RC522_BIT_TRUNNUNG           0x08   /*!< Bit position for status Timer is running. */
#define     RC522_BIT_HIALERT            0x02   /*!< Bit position for status HiAlert. */
#define     RC522_BIT_LOALERT            0x01   /*!< Bit position for status LoAlert. */

/* Status 2 Register				    		(08) */
#define     RC522_BIT_TEMPSENSOFF        0x80   /*!< Bit position to switch Temperture sensors on/off. */
#define     RC522_BIT_I2CFORCEHS         0x40   /*!< Bit position to forece High speed mode for I2C Interface. */
#define     RC522_BIT_CRYPTO1ON          0x08   /*!< Bit position for reader status Crypto is on. */

/* FIFOLevel Register				    		(0A) */
#define     RC522_BIT_FLUSHBUFFER        0x80   /*!< Clears FIFO buffer if set to 1 */

/* Control Register					    		(0C) */
#define     RC522_BIT_TSTOPNOW           0x80   /*!< Stops timer if set to 1. */
#define     RC522_BIT_TSTARTNOW          0x40   /*!< Starts timer if set to 1. */

/* BitFraming Register					    (0D) */
#define     RC522_BIT_STARTSEND          0x80   /*!< Starts transmission in transceive command if set to 1. */

/* BitFraming Register					    (0E) */
#define     RC522_BIT_VALUESAFTERCOLL    0x80   /*!< Activates mode to keep data after collision. */
/*@}*/

/*! \name RC522 Bit definitions of Page 1
 *  \ingroup reg
 *  Below there are useful bit definition of the RC522 register set of Page 1.
 */
/*@{*/
/* Mode Register							(11) */
#define     RC522_BIT_TXWAITRF           0x20   /*!< Tx waits until Rf is enabled until transmit is startet, else
                                                transmit is started immideately. */
#define     RC522_BIT_POLSIGIN           0x08   /*!< Inverts polarity of SiginActIrq, if bit is set to 1 IRQ occures
                                                when Sigin line is 0. */

/* TxMode Register							(12) */
#define     RC522_BIT_INVMOD             0x08   /*!< Activates inverted transmission mode. */

/* RxMode Register							(13) */
#define     RC522_BIT_RXNOERR            0x08   /*!< If 1, receiver does not receive less than 4 bits. */

/* Definitions for Tx and Rx		    (12, 13) */
#define     RC522_BIT_106KBPS            0x00   /*!< Activates speed of 106kbps. */
#define     RC522_BIT_212KBPS            0x10   /*!< Activates speed of 212kbps. */
#define     RC522_BIT_424KBPS            0x20   /*!< Activates speed of 424kbps. */

#define     RC522_BIT_CRCEN              0x80   /*!< Activates transmit or receive CRC. */

/* TxControl Register						(14) */
#define     RC522_BIT_INVTX2ON           0x80   /*!< Inverts the Tx2 output if drivers are switched on. */
#define     RC522_BIT_INVTX1ON           0x40   /*!< Inverts the Tx1 output if drivers are switched on. */
#define     RC522_BIT_INVTX2OFF          0x20   /*!< Inverts the Tx2 output if drivers are switched off. */
#define     RC522_BIT_INVTX1OFF          0x10   /*!< Inverts the Tx1 output if drivers are switched off. */
#define     RC522_BIT_TX2CW              0x08   /*!< Does not modulate the Tx2 output, only constant wave. */
#define     RC522_BIT_TX2RFEN            0x02   /*!< Switches the driver for Tx2 pin on. */
#define     RC522_BIT_TX1RFEN            0x01   /*!< Switches the driver for Tx1 pin on. */

/* Demod Register 							(19) */
#define     RC522_BIT_FIXIQ              0x20   /*!< If set to 1 and the lower bit of AddIQ is set to 0, the receiving is fixed to I channel.
                                                If set to 1 and the lower bit of AddIQ is set to 1, the receiving is fixed to Q channel. */
/*@}*/


/*! \name RC522 Bit definitions of Page 2
 *  \ingroup reg
 *  Below there are useful bit definition of the RC522 register set.
 */
/*@{*/
/* TMode Register 							(2A) */
#define     RC522_BIT_TAUTO              0x80   /*!< Sets the Timer start/stop conditions to Auto mode. */
#define     RC522_BIT_TAUTORESTART       0x10   /*!< Restarts the timer automatically after finished
                                                counting down to 0. */
/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Bitmask Definitions
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name RC522 Bitmask definitions
 *  \ingroup reg
 *  Below there are some useful mask defintions for the RC522. All specified
 *  bits are set to 1.
 */
/*@{*/

/* Command register                 (0x01)*/
#define     RC522_MASK_COMMAND           0x0F   /*!< Bitmask for Command bits in Register RC522_REG_COMMAND. */

/* Waterlevel register              (0x0B)*/
#define     RC522_MASK_WATERLEVEL        0x3F   /*!< Bitmask for Waterlevel bits in register RC522_REG_WATERLEVEL. */

/* Control register                 (0x0C)*/
#define     RC522_MASK_RXBITS            0x07   /*!< Bitmask for RxLast bits in register RC522_REG_CONTROL. */

/* Mode register                    (0x11)*/
#define     RC522_MASK_CRCPRESET         0x03   /*!< Bitmask for CRCPreset bits in register RC522_REG_MODE. */

/* TxMode register                  (0x12, 0x13)*/
#define     RC522_MASK_SPEED             0x70   /*!< Bitmask for Tx/RxSpeed bits in register RC522_REG_TXMODE and RC522_REG_RXMODE. */

/* TxSel register                   (0x16)*/
#define     RC522_MASK_DRIVERSEL         0x30   /*!< Bitmask for DriverSel bits in register RC522_REG_TXSEL. */
#define     RC522_MASK_SIGOUTSEL         0x0F   /*!< Bitmask for SigoutSel bits in register RC522_REG_TXSEL. */

/* RxSel register                   (0x17)*/
#define     RC522_MASK_UARTSEL           0xC0   /*!< Bitmask for UartSel bits in register RC522_REG_RXSEL. */
#define     RC522_MASK_RXWAIT            0x3F   /*!< Bitmask for RxWait bits in register RC522_REG_RXSEL. */

/* RxThreshold register             (0x18)*/
#define     RC522_MASK_MINLEVEL          0xF0   /*!< Bitmask for MinLevel bits in register RC522_REG_RXTHRESHOLD. */
#define     RC522_MASK_COLLEVEL          0x07   /*!< Bitmask for CollLevel bits in register RC522_REG_RXTHRESHOLD. */

/* Demod register                   (0x19)*/
#define     RC522_MASK_ADDIQ             0xC0   /*!< Bitmask for ADDIQ bits in register RC522_REG_DEMOD. */
#define     RC522_MASK_TAURCV            0x0C   /*!< Bitmask for TauRcv bits in register RC522_REG_DEMOD. */
#define     RC522_MASK_TAUSYNC           0x03   /*!< Bitmask for TauSync bits in register RC522_REG_DEMOD. */

/* RFCfg register                   (0x26)*/
#define     RC522_MASK_RXGAIN            0x70   /*!< Bitmask for RxGain bits in register RC522_REG_RFCFG. */

/* GsN register                     (0x27)*/
#define     RC522_MASK_CWGSN             0xF0   /*!< Bitmask for CWGsN bits in register RC522_REG_GSN. */
#define     RC522_MASK_MODGSN            0x0F   /*!< Bitmask for ModGsN bits in register RC522_REG_GSN. */

/* CWGsP register                   (0x28)*/
#define     RC522_MASK_CWGSP             0x3F   /*!< Bitmask for CWGsP bits in register RC522_REG_CWGSP. */

/* ModGsP register                  (0x29)*/
#define     RC522_MASK_MODGSP            0x3F   /*!< Bitmask for ModGsP bits in register RC522_REG_MODGSP. */

/* TMode register                   (0x2A)*/
#define     RC522_MASK_TGATED            0x60   /*!< Bitmask for TGated bits in register RC522_REG_TMODE. */
#define     RC522_MASK_TPRESCALER_HI     0x0F   /*!< Bitmask for TPrescalerHi bits in register RC522_REG_TMODE. */

/*@}*/

#endif /* __RC522REG_H__ */

/* ///////////////////////////////////////////////////////////////////////////
 * End of File
 * //////////////////////////////////////////////////////////////////////// */
