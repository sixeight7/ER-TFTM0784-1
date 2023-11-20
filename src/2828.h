#ifndef __2828_H__
#define __2828_H__

#include <Arduino.h>
#include <stdint.h>

#define DEVICE_IDENTIFICATION_REGISTER 0xB0
#define RGB_INTERFACE_CONTROL_REGISTER_1 0xB1
#define RGB_INTERFACE_CONTROL_REGISTER_2 0xB2
#define RGB_INTERFACE_CONTROL_REGISTER_3 0xB3
#define RGB_INTERFACE_CONTROL_REGISTER_4 0xB4
#define RGB_INTERFACE_CONTROL_REGISTER_5 0xB5
#define RGB_INTERFACE_CONTROL_REGISTER_6 0xB6
#define CONFIGURATION_REGISTER 0xB7
#define VC_CONTROL_REGISTER 0xB8
#define PLL_CONTROL_REGISTER 0xB9
#define PLL_CONFIGURATION_REGISTER 0xBA
#define CLOCK_CONTROL_REGISTER 0xBB
#define PACKET_SIZE_CONTROL_REGISTER_1 0xBC
#define PACKET_SIZE_CONTROL_REGISTER_2 0xBD
#define PACKET_SIZE_CONTROL_REGISTER_3 0xBE
#define PACKET_DROP_REGISTER 0xBF
#define OPERATION_CONTROL_REGISTER 0xC0
#define MAXIMUM_RETURN_SIZE_REGISTER 0xC1
#define RETURN_DATA_COUNT_REGISTER 0xC2
#define ACK_RESPONSE_REGISTER 0xC3
#define LINE_CONTROL_REGISTER 0xC4
#define INTERRUPT_CONTROL_REGISTER 0xC5
#define INTERRUPT_STATUS_REGISTER 0xC6
#define ERROR_STATUS_REGISTER 0xC7
#define DELAY_ADJUSTMENT_REGISTER_1 0xC9
#define DELAY_ADJUSTMENT_REGISTER_2 0xCA
#define DELAY_ADJUSTMENT_REGISTER_3 0xCB
#define DELAY_ADJUSTMENT_REGISTER_4 0xCC
#define DELAY_ADJUSTMENT_REGISTER_5 0xCD
#define DELAY_ADJUSTMENT_REGISTER_6 0xCE
#define HS_TX_TIMER_REGISTER_1 0xCF
#define HS_TX_TIMER_REGISTER_2 0xD0
#define LP_RX_TIMER_REGISTER_1 0xD1
#define LP_RX_TIMER_REGISTER_2 0xD2
#define TE_STATUS_REGISTER 0xD3
#define SPI_READ_REGISTER 0xD4
#define PLL_LOCK_REGISTER 0xD5
#define TEST_REGISTER 0xD6
#define TE_COUNT_REGISTER 0xD7
#define ANALOG_CONTROL_REGISTER_1 0xD8
#define ANALOG_CONTROL_REGISTER_2 0xD9
#define ANALOG_CONTROL_REGISTER_3 0xDA
#define ANALOG_CONTROL_REGISTER_4 0xDB
#define INTERRUPT_OUTPUT_CONTROL_REGISTER 0xDC
#define RGB_INTERFACE_CONTROL_REGISTER_7 0xDD
#define LANE_CONFIGURATION_REGISTER 0xDE
#define DELAY_ADJUSTMENT_REGISTER_7 0xDF
#define PULL_CONTROL_REGISTER_1 0xE0
#define PULL_CONTROL_REGISTER_2 0xE1
#define PULL_CONTROL_REGISTER_3 0xE2
#define CABC_BRIGHTNESS_CONTROL_REGISTER_1 0xE9
#define CABC_BRIGHTNESS_CONTROL_REGISTER_2 0xEA
#define CABC_BRIGHTNESS_STATUS_REGISTER 0xEB
#define ENCODER_CONTROL_REGISTER 0xEC
#define VIDEO_SYNC_DELAY_REGISTER 0xED
#define TRIMMING_REGISTER 0xEE
#define GPIO_REGISTER_1 0xEF
#define GPIO_REGISTER_2 0xF0
#define DLYA01_REGISTER 0xF1
#define DLYA23_REGISTER 0xF2
#define DLYB01_REGISTER 0xF3
#define DLYB23_REGISTER 0xF4
#define DLYC01_REGISTER 0xF5
#define DLYC23_REGISTER 0xF6
#define ANALOG_CONTROL_REGISTER_5 0xF7
#define READ_REGISTER 0xFF

#define LCD_VBPD    10    //    12   // 14
#define LCD_VFPD    12    //    6    // 16
#define LCD_VSPW    2    //    2    //  2
#define LCD_HBPD    160    //    98    // 130
#define LCD_HFPD    160    //    20    // 130
#define LCD_HSPW    24    //    24  // 10

class SSD2828 {
public:
    SSD2828(uint8_t sdi_pin, uint8_t sclk_pin, uint8_t cs_pin, uint8_t rst_pin);
    void Reset(void);
    void LCD_Initial(void);
    void Initial(void);

private:
    void SPI_3W_SET_Cmd(uint16_t Sdata);
    void SPI_3W_SET_PAs(uint16_t Sdata);
    uint8_t SPI_ReadData(void);
    void SPI_WriteData(uint8_t value);
    void SPI_WriteCmd(uint8_t value);
    void GP_COMMAD_PA(uint16_t num);
    void spi_delay(void);

    uint8_t CS_2828;
    uint8_t RST_2828;
    uint8_t SDI_2828;
    uint8_t SCLK_2828;
};

#endif
