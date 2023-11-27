///////////////////////////////////////////////////////////////////////////////
//
// SSD2828 MIPI Bridge - Initialization
//
// Hard-coded to work with this particular LCD panel:
//
// Brand: EASTRISING
// Model: ER-TFTM0784-1
//        ( 400 x 1280 dots 7.84" color IPS tft lcd )
//
// This code was only slightly modified from the 8051 example code
// written by JAVEN LIU. In his example, he bit-bangs the SPI to the
// SSD2828, and since this only needs to be done on powerup and doesnt
// need to be performant,  I decided to keep it that way to save resources.
//
// I did change the GPIO stuff to be Arduino-oriented rather than Keil 8051.
//
///////////////////////////////////////////////////////////////////////////////
#include "2828.h"
//-----------------------------------------------------------------------------
SSD2828::SSD2828(uint8_t sdi_pin, uint8_t sclk_pin, uint8_t cs_pin, uint8_t rst_pin) {
  SDI_2828 = sdi_pin;
  SCLK_2828 = sclk_pin;
  CS_2828 = cs_pin;
  RST_2828 = rst_pin;
}

void SSD2828::Reset(void)
{

  // GPIO init

  pinMode(SDI_2828, OUTPUT);
  digitalWrite(SDI_2828, 1);

  pinMode(SCLK_2828, OUTPUT);
  digitalWrite(SCLK_2828, 1);

  pinMode(CS_2828, OUTPUT);
  digitalWrite(CS_2828, 1);

  pinMode(RST_2828, OUTPUT);
  digitalWrite(RST_2828, 1);

  // chip reset

  digitalWrite(RST_2828, 0);
  delay(30);

  digitalWrite(RST_2828, 1);
  delay(50);

}
//-----------------------------------------------------------------------------
inline void SSD2828::spi_delay(void)
{
  // 128 NOPS should bring the SPI speed well below 2.3 MHz on
  // a Teensy 4.0. On AVR devices, even slower.

  // Source: https://forum.pjrc.com/threads/42865-A-set-of-scope-tested-100-500-ns-delay-macros
  for (uint32_t i = 0; i < 50; i++) __asm__("nop\n\t"); // Delay of 213 ns
}
//-----------------------------------------------------------------------------
void SSD2828::SPI_3W_SET_Cmd(uint16_t Sdata)
{
  uint8_t i;
  digitalWrite(CS_2828, 0);
  spi_delay();
  digitalWrite(SDI_2828, 0);
  spi_delay();
  digitalWrite(SCLK_2828 , 0);
  spi_delay();
  digitalWrite(SCLK_2828 , 1);
  spi_delay();
  for (i = 8; i > 0; i--)
  {
    if (Sdata & 0x80)
      digitalWrite(SDI_2828, 1);
    else
      digitalWrite(SDI_2828, 0);
    spi_delay();
    digitalWrite(SCLK_2828 , 0);
    spi_delay();
    digitalWrite(SCLK_2828 , 1);
    spi_delay();
    Sdata <<= 1;
  }
  digitalWrite(SCLK_2828 , 0);
  spi_delay();
  digitalWrite(CS_2828, 1);
  spi_delay();
}
//-----------------------------------------------------------------------------
void SSD2828::SPI_3W_SET_PAs(uint16_t Sdata)
{
  uint8_t i;
  digitalWrite(CS_2828, 0);
  spi_delay();
  digitalWrite(SDI_2828, 1);
  spi_delay();
  digitalWrite(SCLK_2828 , 0);
  spi_delay();
  digitalWrite(SCLK_2828 , 1);
  spi_delay();
  for (i = 8; i > 0; i--)
  {
    if (Sdata & 0x80)
      digitalWrite(SDI_2828, 1);
    else
      digitalWrite(SDI_2828, 0);
    spi_delay();
    digitalWrite(SCLK_2828 , 0);
    spi_delay();
    digitalWrite(SCLK_2828 , 1);
    spi_delay();
    Sdata <<= 1;
  }
  digitalWrite(SCLK_2828 , 0);
  spi_delay();
  digitalWrite(CS_2828, 1);
  spi_delay();
}
//-----------------------------------------------------------------------------
void SSD2828::SPI_WriteData(uint8_t value)
{
  SPI_3W_SET_PAs(value);
}
//-----------------------------------------------------------------------------
void SSD2828::SPI_WriteCmd(uint8_t value)
{
  SPI_3W_SET_Cmd(value);
}
//-----------------------------------------------------------------------------
void SSD2828::GP_COMMAD_PA(uint16_t num)
{
  SPI_WriteCmd(PACKET_SIZE_CONTROL_REGISTER_1);
  SPI_WriteData(num & 0xff);
  SPI_WriteData((num >> 8) & 0xff);
  SPI_WriteCmd(PACKET_DROP_REGISTER);
}
//-----------------------------------------------------------------------------
void SSD2828::SendCmd(uint8_t cmd, uint8_t data1, uint8_t data2) {
    SPI_WriteCmd(cmd);
    SPI_WriteData(data1);
    SPI_WriteData(data2);
}

void SSD2828::WriteDataPacket(uint16_t num, ...) {
    va_list args;
    va_start(args, num);

    SPI_WriteCmd(PACKET_SIZE_CONTROL_REGISTER_1);
    SPI_WriteData(num & 0xff);
    SPI_WriteData((num >> 8) & 0xff);
    SPI_WriteCmd(PACKET_DROP_REGISTER);

    for (uint16_t i = 0; i < num; ++i) {
        uint8_t data = va_arg(args, int);
        SPI_WriteData(data);
    }
    
    va_end(args);
}

void SSD2828::Initial(void)
{
    SendCmd(CONFIGURATION_REGISTER, 0x50, 0x00);  // 50=TX_CLK 70=PCLK
    SendCmd(VC_CONTROL_REGISTER, 0x00, 0x00);  // VC(Virtual ChannelID) Control Register
    SendCmd(PLL_CONTROL_REGISTER, 0x00, 0x00);  // 1=PLL disable
    SendCmd(PLL_CONFIGURATION_REGISTER, 0x20, 0x82);  // NS=1, PLL00=62.5-125, MS=1
    SendCmd(CLOCK_CONTROL_REGISTER, 0x07, 0x00);  // LP Clock Divider LP clock = 400MHz / LPD / 8 = 480 / 8/ 8 = 7MHz
    SendCmd(PLL_CONTROL_REGISTER, 0x01, 0x00);  // 1=PLL disable

    WriteDataPacket(1, 0x10);
    WriteDataPacket(2, 0xCD, 0xAA);
    WriteDataPacket(2, 0x41, 0x34);
    WriteDataPacket(2, 0x30, 0x00);
    WriteDataPacket(2, 0x39, 0x11);
    WriteDataPacket(2, 0x32, 0x00);
    WriteDataPacket(2, 0x33, 0x38);
    WriteDataPacket(2, 0x35, 0x24);
    WriteDataPacket(2, 0x4F, 0x35);
    WriteDataPacket(2, 0x4E, 0x27);
    WriteDataPacket(2, 0x41, 0x56);
    WriteDataPacket(9, 0x55, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F);
    WriteDataPacket(17, 0x56, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F);
    WriteDataPacket(2, 0x65, 0x08);
    WriteDataPacket(2, 0x3A, 0x08);
    WriteDataPacket(2, 0x36, 0x49);
    WriteDataPacket(2, 0x67, 0x82);
    WriteDataPacket(2, 0x69, 0x20);
    WriteDataPacket(2, 0x6C, 0x80);
    WriteDataPacket(2, 0x6D, 0x01);
    WriteDataPacket(20, 0x53, 0x1F, 0x19, 0x15, 0x11, 0x11, 0x11, 0x12, 0x14, 0x15, 0x11, 0x0D, 0x0B, 0x0B, 0x0D, 0x0C, 0x0C, 0x08, 0x04, 0x00);
    WriteDataPacket(20, 0x54, 0x1F, 0x19, 0x15, 0x11, 0x11, 0x11, 0x13, 0x15, 0x16, 0x11, 0x0D, 0x0C, 0x0C, 0x0E, 0x0C, 0x0C, 0x08, 0x04, 0x00);
    WriteDataPacket(2, 0x6B, 0x00);
    WriteDataPacket(2, 0x58, 0x00);
    WriteDataPacket(2, 0x73, 0xF0);
    WriteDataPacket(2, 0x76, 0x40);
    WriteDataPacket(2, 0x77, 0x04);
    WriteDataPacket(2, 0x74, 0x17);
    WriteDataPacket(2, 0x5E, 0x03);
    WriteDataPacket(2, 0x68, 0x10);
    WriteDataPacket(2, 0x6A, 0x00);
    WriteDataPacket(2, 0x28, 0x31);
    WriteDataPacket(2, 0x29, 0x21);
    WriteDataPacket(2, 0x63, 0x04);
    WriteDataPacket(2, 0x27, 0x00);
    WriteDataPacket(2, 0x7C, 0x80);
    WriteDataPacket(2, 0x2E, 0x05);
    WriteDataPacket(2, 0x4C, 0x80);
    WriteDataPacket(2, 0x50, 0xC0);
    WriteDataPacket(2, 0x78, 0x6E);
    WriteDataPacket(2, 0x2D, 0x31);
    WriteDataPacket(2, 0x49, 0x00);
    WriteDataPacket(2, 0x4D, 0x00);
    WriteDataPacket(1, 0x11);
    delay(120);
    WriteDataPacket(1, 0x29);
    delay(120);

    SendCmd(CONFIGURATION_REGISTER, 0x50, 0x00);  // Configuration Register
    SendCmd(VC_CONTROL_REGISTER, 0x00, 0x00);  // VC(Virtual ChannelID) Control Register
    SendCmd(PLL_CONTROL_REGISTER, 0x00, 0x00);  // 1=PLL disable
    SendCmd(PLL_CONFIGURATION_REGISTER, 0x20, 0x82);  // NS=1, PLL00=62.5-125, MS=1
    SendCmd(CLOCK_CONTROL_REGISTER, 0x07, 0x00);  // LP Clock Divider LP clock = 400MHz / LPD / 8 = 480 / 8/ 8 = 7MHz
    SendCmd(PLL_CONTROL_REGISTER, 0x01, 0x00);  // 1=PLL disable
    SendCmd(DELAY_ADJUSTMENT_REGISTER_1, 0x02, 0x23);  // p1: HS-Data-zero  p2: HS-Data- prepare  --> 8031 issue
    delay(100);

    SendCmd(DELAY_ADJUSTMENT_REGISTER_2, 0x01, 0x23);  // CLK Prepare, Clk Zero
    SendCmd(DELAY_ADJUSTMENT_REGISTER_3, 0x10, 0x05);  // Clk Post, Clk Per
    SendCmd(DELAY_ADJUSTMENT_REGISTER_4, 0x05, 0x10);  // HS Trail, Clk Trail
    SendCmd(LP_RX_TIMER_REGISTER_2, 0x00, 0x00);


    //RGB interface configuration
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_1, 0x18, 0x02);  // HSPW 07, VSPW 05
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_2, 0xA0, 0x0A);  // HBPD 0x64=100, VBPD 8
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_3, 0xA0, 0x0C);  // HFPD 8, VFPD 10
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_4, 0x90, 0x01);  // Horizontal active period 720=02D0, 013F=319 02D0=720
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_5, 0x00, 0x05);  // Vertical active period 1280=0500
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_6, 0x0B, 0x00);  // RGB CLK 16BPP=00 18BPP=01, D7=0 D6=0 D5=0  D1-0=11 ¨C 24bpp, D15=VS D14=HS D13=CLK D12-9=NC D8=0=Video with blanking packet. 00-F0


    //MIPI lane configuration
    SendCmd(LANE_CONFIGURATION_REGISTER, 0x03, 0x00);  // 11=4LANE 10=3LANE 01=2LANE 00=1LANE
    SendCmd(RGB_CONTROL_REGISTER, 0x05, 0x00);  // 05=BGR 04=RGB, D0=0=RGB 1:BGR D1=1=Most significant byte sent first
    SendCmd(PULL_CONTROL_REGISTER_3, 0x58, 0x00);
    SendCmd(CONFIGURATION_REGISTER, 0x6B, 0x02);
    SendCmd(CONFIGURATION_REGISTER, 0x50, 0x00); //50=TX_CLK 70=PCLK
    SendCmd(VC_CONTROL_REGISTER, 0x00, 0x00);  // VC(Virtual ChannelID) Control Register
    SendCmd(PLL_CONTROL_REGISTER, 0x00, 0x00);  // 1=PLL disable
    SendCmd(PLL_CONFIGURATION_REGISTER, 0x14, 0x42);  //0x14,D7-0=NS(0x01 : NS=1), 0x42,D15-14=PLL00=62.5-125 01=126-250 10=251-500 11=501-1000  DB12-8=MS(01:MS=1)
    SendCmd(CLOCK_CONTROL_REGISTER, 0x03, 0x00);  // LP Clock Divider LP clock = 400MHz / LPD / 8 = 480 / 8/ 8 = 7MHz
    SendCmd(PLL_CONTROL_REGISTER, 0x01, 0x00);  // 1=PLL disable
    
    // MIPI lane configuration
    SendCmd(LANE_CONFIGURATION_REGISTER, 0x00, 0x00);
    SendCmd(DELAY_ADJUSTMENT_REGISTER_1, 0x02, 0x23);  // p1: HS-Data-zero  p2: HS-Data- prepare  --> 8031 issue

  //    delay(100);

  //    LCD_initial();
    SendCmd(CONFIGURATION_REGISTER, 0x50, 0x00);
    SendCmd(VC_CONTROL_REGISTER, 0x00, 0x00);  // VC(Virtual ChannelID) Control Register
    SendCmd(PLL_CONTROL_REGISTER, 0x00, 0x00);  // 1=PLL disable
    SendCmd(PLL_CONFIGURATION_REGISTER, 0x2D, 0x82);  // NS=1, PLL00=62.5-125, MS=1
    SendCmd(CLOCK_CONTROL_REGISTER, 0x07, 0x00);  // LP Clock Divider LP clock = 400MHz / LPD / 8 = 480 / 8/ 8 = 7MHz
    SendCmd(PLL_CONTROL_REGISTER, 0x01, 0x00);  // 1=PLL disable
    SendCmd(DELAY_ADJUSTMENT_REGISTER_1, 0x02, 0x23);  // p1: HS-Data-zero  p2: HS-Data- prepare  --> 8031 issue
    delay(100);
    
    SendCmd(DELAY_ADJUSTMENT_REGISTER_2, 0x01, 0x23);  // CLK Prepare, Clk Zero
    SendCmd(DELAY_ADJUSTMENT_REGISTER_3, 0x10, 0x05);  // local_write_reg(addr=0xCB,data=0x0510), Clk Post, Clk Per
    SendCmd(DELAY_ADJUSTMENT_REGISTER_4, 0x05, 0x10);  // local_write_reg(addr=0xCC,data=0x100A), HS Trail, Clk Trail
    SendCmd(LP_RX_TIMER_REGISTER_2, 0x00, 0x00);

    /*
      // RGB interface configuration
      SendCmd(RGB_INTERFACE_CONTROL_REGISTER_1, 0x18, 0x02);  // HSPW 07, VSPW 05
      SendCmd(RGB_INTERFACE_CONTROL_REGISTER_2, 0xA0, 0x0A);  // HBPD 0x64=100, VBPD 8
      SendCmd(RGB_INTERFACE_CONTROL_REGISTER_3, 0xA0, 0x0C);  // HFPD 8, VFPD 10
    */

    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_1, LCD_HSPW, LCD_VSPW);  // HSA, VSA
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_2, LCD_HBPD, LCD_VBPD);  // HBP, VBP
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_3, LCD_HFPD, LCD_VFPD);  // HFP, VFP
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_4, 0x90, 0x01);  // Horizontal active period 400, 013F=319 02D0=72
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_5, 0x00, 0x05);  // Vertical active period 1280
    SendCmd(RGB_INTERFACE_CONTROL_REGISTER_6, 0x0B, 0xC0);  // RGB CLK 16BPP=00 18BPP=01 / D7=0 D6=0 D5=0  D1-0=11 ®C 24bpp / D15=VS D14=HS D13=CLK D12-9=NC D8=0=Video with blanking packet. 00-F0
    
    // MIPI lane configuration
    SendCmd(LANE_CONFIGURATION_REGISTER, 0x03, 0x00); //11=4LANE 10=3LANE 01=2LANE 00=1LANE
    SendCmd(RGB_CONTROL_REGISTER, 0x01, 0x00);  // 05=BGR 04=RGB, D0=0=RGB 1:BGR D1=1=Most significant byte sent first
    SendCmd(PULL_CONTROL_REGISTER_3, 0x58, 0x00);
    SendCmd(CONFIGURATION_REGISTER, 0x4B, 0x02);
    SPI_WriteCmd(0x2c);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

