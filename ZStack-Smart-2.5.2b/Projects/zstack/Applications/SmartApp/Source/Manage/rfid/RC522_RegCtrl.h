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
 #ifndef  __REGCTRL_H__
 #define __REGCTRL_H__


    #define MFRC522_I2C_RDAdr 0x53
    #define MFRC522_I2C_WRAdr 0x52  
    
//    #define ACK   0
    #define NACK  1
    
    #define OK     0
    #define ERROR  -1

     /*************************************************
     Function:       RcSetReg
     Description:
          Write data to register of RC522
     Parameter:
          RegAddr       The address of the regitster
          RegVal        The value to be writen
     Return:
          None
     **************************************************/
    /*! \ingroup my_regs
        \param[in]  RegAddr       The address of the regitster
        \param[in]  RegVal        The value to be writen
        \brief Write data to register of RC522.
     */
    void RcSetReg(unsigned char RegAddr, unsigned char RegVal);
    
     /*************************************************
     Function:       RcGetReg
     Description:
          Read data from a register of RC522.
     Parameter:
          RegAddr       The address of the regitster to be readed
     Return:
          The value of the specify register
     **************************************************/
    /*! \ingroup my_regs
        \param[in]  RegAddr       The address of the regitster
        \return     The value of the specify register
        \brief Read data from a register of RC522.
     */
     unsigned char RcGetReg(unsigned char RegAddr);
    
     void RcModifyReg(unsigned char RegAddr, unsigned char ModifyVal, unsigned char MaskByte);

#endif