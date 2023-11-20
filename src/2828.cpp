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

  pinMode(SDI_2828,OUTPUT);
  digitalWrite(SDI_2828,1);

  pinMode(SCLK_2828,OUTPUT);
  digitalWrite(SCLK_2828,1);

  pinMode(CS_2828,OUTPUT);
  digitalWrite(CS_2828,1);

  pinMode(RST_2828,OUTPUT);
  digitalWrite(RST_2828,1);

  // chip reset

  digitalWrite(RST_2828,0);
  delay(30);

  digitalWrite(RST_2828,1);
  delay(50);
    
  digitalWrite(SCLK_2828,0);

}
//-----------------------------------------------------------------------------
inline void SSD2828::spi_delay(void)
{
  // 128 NOPS should bring the SPI speed well below 2.3 MHz on 
  // a Teensy 4.0. On AVR devices, even slower.

  // Source: https://forum.pjrc.com/threads/42865-A-set-of-scope-tested-100-500-ns-delay-macros
  for (uint32_t i=0; i<50; i++) __asm__("nop\n\t");  // Delay of 213 ns
}
//-----------------------------------------------------------------------------
void SSD2828::SPI_3W_SET_Cmd(uint16_t Sdata)
{ 
  uint8_t i;
  digitalWrite(CS_2828,0);
  spi_delay();
  digitalWrite(SDI_2828,0);
  spi_delay();
  digitalWrite(SCLK_2828 ,0); 
  spi_delay();
  digitalWrite(SCLK_2828 ,1);
  spi_delay();
  for(i = 8; i > 0; i--)
  {
    if(Sdata & 0x80)
      digitalWrite(SDI_2828,1);
    else
      digitalWrite(SDI_2828,0);
    spi_delay();
    digitalWrite(SCLK_2828 ,0); 
    spi_delay();
    digitalWrite(SCLK_2828 ,1);
    spi_delay();
    Sdata <<= 1;
  }
  digitalWrite(SCLK_2828 ,0);
  spi_delay();  
  digitalWrite(CS_2828,1);
  spi_delay();  
}
//-----------------------------------------------------------------------------
void SSD2828::SPI_3W_SET_PAs(uint16_t Sdata)
{
  uint8_t i;
  digitalWrite(CS_2828,0);
  spi_delay();  
  digitalWrite(SDI_2828,1);
  spi_delay();  
  digitalWrite(SCLK_2828 ,0);
  spi_delay();  
  digitalWrite(SCLK_2828 ,1);
  spi_delay();  
  for(i = 8; i > 0; i--)
  {
    if(Sdata & 0x80)
      digitalWrite(SDI_2828,1);
    else
      digitalWrite(SDI_2828,0);
    spi_delay();    
    digitalWrite(SCLK_2828 ,0); 
    spi_delay();    
    digitalWrite(SCLK_2828 ,1);
    spi_delay();
    Sdata <<= 1;
  }
  digitalWrite(SCLK_2828 ,0);
  spi_delay();
  digitalWrite(CS_2828,1);
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
  SPI_WriteCmd(PACKET_SIZE_CONTROL_REGISTER_1); // Packet Size Control Register 1
  SPI_WriteData(num & 0xff);
  SPI_WriteData((num >> 8)&0xff);
  SPI_WriteCmd(PACKET_DROP_REGISTER); // Packet Drop Register
}
//-----------------------------------------------------------------------------
void SSD2828::LCD_Initial(void)
{
    GP_COMMAD_PA(2); SPI_WriteData(0xB0);SPI_WriteData(0x5A);  //(0xB0,0x5A);

    GP_COMMAD_PA(2); SPI_WriteData(0xB1);SPI_WriteData(0x00);  //(0xB1,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x89);SPI_WriteData(0x01);  //(0x89,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x91);SPI_WriteData(0x17);  //(0x91,0x17);
    GP_COMMAD_PA(2); SPI_WriteData(0xB1);SPI_WriteData(0x03);  //(0xB1,0x03);
    GP_COMMAD_PA(2); SPI_WriteData(0x2C);SPI_WriteData(0x28);  //(0x2C,0x28);

    GP_COMMAD_PA(2); SPI_WriteData(0x00);SPI_WriteData(0xB7);  //(0x00,0xB7);
    GP_COMMAD_PA(2); SPI_WriteData(0x01);SPI_WriteData(0x1B);  //(0x01,0x1B);
    GP_COMMAD_PA(2); SPI_WriteData(0x02);SPI_WriteData(0x00);  //(0x02,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x03);SPI_WriteData(0x00);  //(0x03,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x04);SPI_WriteData(0x00);  //(0x04,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x05);SPI_WriteData(0x00);  //(0x05,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x06);SPI_WriteData(0x00);  //(0x06,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x07);SPI_WriteData(0x00);  //(0x07,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x08);SPI_WriteData(0x00);  //(0x08,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x09);SPI_WriteData(0x00);  //(0x09,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x0A);SPI_WriteData(0x01);  //(0x0A,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x0B);SPI_WriteData(0x01);  //(0x0B,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x0C);SPI_WriteData(0x00);  //(0x0C,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x0D);SPI_WriteData(0x00);  //(0x0D,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x0E);SPI_WriteData(0x24);  //(0x0E,0x24);
    GP_COMMAD_PA(2); SPI_WriteData(0x0F);SPI_WriteData(0x1C);  //(0x0F,0x1C);
    GP_COMMAD_PA(2); SPI_WriteData(0x10);SPI_WriteData(0xC9);  //(0x10,0xC9);
    GP_COMMAD_PA(2); SPI_WriteData(0x11);SPI_WriteData(0x60);  //(0x11,0x60);
    GP_COMMAD_PA(2); SPI_WriteData(0x12);SPI_WriteData(0x70);  //(0x12,0x70);
    GP_COMMAD_PA(2); SPI_WriteData(0x13);SPI_WriteData(0x01);  //(0x13,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x14);SPI_WriteData(0xE7);  //(0x14,0xE7);
    GP_COMMAD_PA(2); SPI_WriteData(0x15);SPI_WriteData(0xFF);  //(0x15,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x16);SPI_WriteData(0x3D);  //(0x16,0x3D);
    GP_COMMAD_PA(2); SPI_WriteData(0x17);SPI_WriteData(0x0E);  //(0x17,0x0E);
    GP_COMMAD_PA(2); SPI_WriteData(0x18);SPI_WriteData(0x01);  //(0x18,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x19);SPI_WriteData(0x00);  //(0x19,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x1A);SPI_WriteData(0x00);  //(0x1A,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x1B);SPI_WriteData(0xFC);  //(0x1B,0xFC);
    GP_COMMAD_PA(2); SPI_WriteData(0x1C);SPI_WriteData(0x0B);  //(0x1C,0x0B);
    GP_COMMAD_PA(2); SPI_WriteData(0x1D);SPI_WriteData(0xA0);  //(0x1D,0xA0);
    GP_COMMAD_PA(2); SPI_WriteData(0x1E);SPI_WriteData(0x03);  //(0x1E,0x03);
    GP_COMMAD_PA(2); SPI_WriteData(0x1F);SPI_WriteData(0x04);  //(0x1F,0x04);
    GP_COMMAD_PA(2); SPI_WriteData(0x20);SPI_WriteData(0x0C);  //(0x20,0x0C);
    GP_COMMAD_PA(2); SPI_WriteData(0x21);SPI_WriteData(0x00);  //(0x21,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x22);SPI_WriteData(0x04);  //(0x22,0x04);
    GP_COMMAD_PA(2); SPI_WriteData(0x23);SPI_WriteData(0x81);  //(0x23,0x81);
    GP_COMMAD_PA(2); SPI_WriteData(0x24);SPI_WriteData(0x1F);  //(0x24,0x1F);
    GP_COMMAD_PA(2); SPI_WriteData(0x25);SPI_WriteData(0x10);  //(0x25,0x10);
    GP_COMMAD_PA(2); SPI_WriteData(0x26);SPI_WriteData(0x9B);  //(0x26,0x9B);
    GP_COMMAD_PA(2); SPI_WriteData(0x2D);SPI_WriteData(0x01);  //(0x2D,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x2E);SPI_WriteData(0x84);  //(0x2E,0x84);
    GP_COMMAD_PA(2); SPI_WriteData(0x2F);SPI_WriteData(0x00);  //(0x2F,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x30);SPI_WriteData(0x02);  //(0x30,0x02);
    GP_COMMAD_PA(2); SPI_WriteData(0x31);SPI_WriteData(0x08);  //(0x31,0x08);
    GP_COMMAD_PA(2); SPI_WriteData(0x32);SPI_WriteData(0x01);  //(0x32,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x33);SPI_WriteData(0x1C);  //(0x33,0x1C);
    GP_COMMAD_PA(2); SPI_WriteData(0x34);SPI_WriteData(0x40);  //(0x34,0x40);
    GP_COMMAD_PA(2); SPI_WriteData(0x35);SPI_WriteData(0xFF);  //(0x35,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x36);SPI_WriteData(0xFF);  //(0x36,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x37);SPI_WriteData(0xFF);  //(0x37,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x38);SPI_WriteData(0xFF);  //(0x38,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x39);SPI_WriteData(0xFF);  //(0x39,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x3A);SPI_WriteData(0x05);  //(0x3A,0x05);
    GP_COMMAD_PA(2); SPI_WriteData(0x3B);SPI_WriteData(0x00);  //(0x3B,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x3C);SPI_WriteData(0x00);  //(0x3C,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x3D);SPI_WriteData(0x00);  //(0x3D,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x3E);SPI_WriteData(0xCF);  //(0x3E,0xCF);
    GP_COMMAD_PA(2); SPI_WriteData(0x3F);SPI_WriteData(0x84);  //(0x3F,0x84);
    GP_COMMAD_PA(2); SPI_WriteData(0x40);SPI_WriteData(0x28);  //(0x40,0x28);
    GP_COMMAD_PA(2); SPI_WriteData(0x41);SPI_WriteData(0xFC);  //(0x41,0xFC);
    GP_COMMAD_PA(2); SPI_WriteData(0x42);SPI_WriteData(0x01);  //(0x42,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x43);SPI_WriteData(0x40);  //(0x43,0x40);
    GP_COMMAD_PA(2); SPI_WriteData(0x44);SPI_WriteData(0x05);  //(0x44,0x05);
    GP_COMMAD_PA(2); SPI_WriteData(0x45);SPI_WriteData(0xE8);  //(0x45,0xE8);
    GP_COMMAD_PA(2); SPI_WriteData(0x46);SPI_WriteData(0x16);  //(0x46,0x16);
    GP_COMMAD_PA(2); SPI_WriteData(0x47);SPI_WriteData(0x00);  //(0x47,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x48);SPI_WriteData(0x00);  //(0x48,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x49);SPI_WriteData(0x88);  //(0x49,0x88);
    GP_COMMAD_PA(2); SPI_WriteData(0x4A);SPI_WriteData(0x08);  //(0x4A,0x08);
    GP_COMMAD_PA(2); SPI_WriteData(0x4B);SPI_WriteData(0x05);  //(0x4B,0x05);
    GP_COMMAD_PA(2); SPI_WriteData(0x4C);SPI_WriteData(0x03);  //(0x4C,0x03);
    GP_COMMAD_PA(2); SPI_WriteData(0x4D);SPI_WriteData(0xD0);  //(0x4D,0xD0);
    GP_COMMAD_PA(2); SPI_WriteData(0x4E);SPI_WriteData(0x13);  //(0x4E,0x13);
    GP_COMMAD_PA(2); SPI_WriteData(0x4F);SPI_WriteData(0xFF);  //(0x4F,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x50);SPI_WriteData(0x0A);  //(0x50,0x0A);
    GP_COMMAD_PA(2); SPI_WriteData(0x51);SPI_WriteData(0x53);  //(0x51,0x53);
    GP_COMMAD_PA(2); SPI_WriteData(0x52);SPI_WriteData(0x26);  //(0x52,0x26);
    GP_COMMAD_PA(2); SPI_WriteData(0x53);SPI_WriteData(0x22);  //(0x53,0x22);
    GP_COMMAD_PA(2); SPI_WriteData(0x54);SPI_WriteData(0x09);  //(0x54,0x09);
    GP_COMMAD_PA(2); SPI_WriteData(0x55);SPI_WriteData(0x22);  //(0x55,0x22);
    GP_COMMAD_PA(2); SPI_WriteData(0x56);SPI_WriteData(0x00);  //(0x56,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x57);SPI_WriteData(0x1C);  //(0x57,0x1C);
    GP_COMMAD_PA(2); SPI_WriteData(0x58);SPI_WriteData(0x03);  //(0x58,0x03);
    GP_COMMAD_PA(2); SPI_WriteData(0x59);SPI_WriteData(0x3F);  //(0x59,0x3F);
    GP_COMMAD_PA(2); SPI_WriteData(0x5A);SPI_WriteData(0x28);  //(0x5A,0x28);
    GP_COMMAD_PA(2); SPI_WriteData(0x5B);SPI_WriteData(0x01);  //(0x5B,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x5C);SPI_WriteData(0xCC);  //(0x5C,0xCC);
    GP_COMMAD_PA(2); SPI_WriteData(0x5D);SPI_WriteData(0x21);  //(0x5D,0x21);
    GP_COMMAD_PA(2); SPI_WriteData(0x5E);SPI_WriteData(0x84);  //(0x5E,0x84);
    GP_COMMAD_PA(2); SPI_WriteData(0x5F);SPI_WriteData(0x10);  //(0x5F,0x10);
    GP_COMMAD_PA(2); SPI_WriteData(0x60);SPI_WriteData(0x42);  //(0x60,0x42);
    GP_COMMAD_PA(2); SPI_WriteData(0x61);SPI_WriteData(0x40);  //(0x61,0x40);
    GP_COMMAD_PA(2); SPI_WriteData(0x62);SPI_WriteData(0x06);  //(0x62,0x06);
    GP_COMMAD_PA(2); SPI_WriteData(0x63);SPI_WriteData(0x3A);  //(0x63,0x3A);
    GP_COMMAD_PA(2); SPI_WriteData(0x64);SPI_WriteData(0xA6);  //(0x64,0xA6);
    GP_COMMAD_PA(2); SPI_WriteData(0x65);SPI_WriteData(0x04);  //(0x65,0x04);
    GP_COMMAD_PA(2); SPI_WriteData(0x66);SPI_WriteData(0x09);  //(0x66,0x09);
    GP_COMMAD_PA(2); SPI_WriteData(0x67);SPI_WriteData(0x21);  //(0x67,0x21);
    GP_COMMAD_PA(2); SPI_WriteData(0x68);SPI_WriteData(0x84);  //(0x68,0x84);
    GP_COMMAD_PA(2); SPI_WriteData(0x69);SPI_WriteData(0x10);  //(0x69,0x10);
    GP_COMMAD_PA(2); SPI_WriteData(0x6A);SPI_WriteData(0x42);  //(0x6A,0x42);
    GP_COMMAD_PA(2); SPI_WriteData(0x6B);SPI_WriteData(0x08);  //(0x6B,0x08);
    GP_COMMAD_PA(2); SPI_WriteData(0x6C);SPI_WriteData(0x21);  //(0x6C,0x21);
    GP_COMMAD_PA(2); SPI_WriteData(0x6D);SPI_WriteData(0x84);  //(0x6D,0x84);
    GP_COMMAD_PA(2); SPI_WriteData(0x6E);SPI_WriteData(0x74);  //(0x6E,0x74);
    GP_COMMAD_PA(2); SPI_WriteData(0x6F);SPI_WriteData(0xE2);  //(0x6F,0xE2);
    GP_COMMAD_PA(2); SPI_WriteData(0x70);SPI_WriteData(0x6B);  //(0x70,0x6B);
    GP_COMMAD_PA(2); SPI_WriteData(0x71);SPI_WriteData(0x6B);  //(0x71,0x6B);
    GP_COMMAD_PA(2); SPI_WriteData(0x72);SPI_WriteData(0x94);  //(0x72,0x94);
    GP_COMMAD_PA(2); SPI_WriteData(0x73);SPI_WriteData(0x10);  //(0x73,0x10);
    GP_COMMAD_PA(2); SPI_WriteData(0x74);SPI_WriteData(0x42);  //(0x74,0x42);
    GP_COMMAD_PA(2); SPI_WriteData(0x75);SPI_WriteData(0x08);  //(0x75,0x08);
    GP_COMMAD_PA(2); SPI_WriteData(0x76);SPI_WriteData(0x00);  //(0x76,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x77);SPI_WriteData(0x00);  //(0x77,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x78);SPI_WriteData(0x0F);  //(0x78,0x0F);
    GP_COMMAD_PA(2); SPI_WriteData(0x79);SPI_WriteData(0xE0);  //(0x79,0xE0);
    GP_COMMAD_PA(2); SPI_WriteData(0x7A);SPI_WriteData(0x01);  //(0x7A,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x7B);SPI_WriteData(0xFF);  //(0x7B,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x7C);SPI_WriteData(0xFF);  //(0x7C,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x7D);SPI_WriteData(0x0F);  //(0x7D,0x0F);
    GP_COMMAD_PA(2); SPI_WriteData(0x7E);SPI_WriteData(0x41);  //(0x7E,0x41);
    GP_COMMAD_PA(2); SPI_WriteData(0x7F);SPI_WriteData(0xFE);  //(0x7F,0xFE);


    GP_COMMAD_PA(2); SPI_WriteData(0xB1);SPI_WriteData(0x02);  //(0xB1,0x02);

    GP_COMMAD_PA(2); SPI_WriteData(0x00);SPI_WriteData(0xFF);  //(0x00,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x01);SPI_WriteData(0x05);  //(0x01,0x05);
    GP_COMMAD_PA(2); SPI_WriteData(0x02);SPI_WriteData(0xC8);  //(0x02,0xC8);
    GP_COMMAD_PA(2); SPI_WriteData(0x03);SPI_WriteData(0x00);  //(0x03,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x04);SPI_WriteData(0x14);  //(0x04,0x14);
    GP_COMMAD_PA(2); SPI_WriteData(0x05);SPI_WriteData(0x4B);  //(0x05,0x4B);
    GP_COMMAD_PA(2); SPI_WriteData(0x06);SPI_WriteData(0x64);  //(0x06,0x64);
    GP_COMMAD_PA(2); SPI_WriteData(0x07);SPI_WriteData(0x0A);  //(0x07,0x0A);
    GP_COMMAD_PA(2); SPI_WriteData(0x08);SPI_WriteData(0xC0);  //(0x08,0xC0);
    GP_COMMAD_PA(2); SPI_WriteData(0x09);SPI_WriteData(0x00);  //(0x09,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x0A);SPI_WriteData(0x00);  //(0x0A,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x0B);SPI_WriteData(0x10);  //(0x0B,0x10);
    GP_COMMAD_PA(2); SPI_WriteData(0x0C);SPI_WriteData(0xE6);  //(0x0C,0xE6);
    GP_COMMAD_PA(2); SPI_WriteData(0x0D);SPI_WriteData(0x0D);  //(0x0D,0x0D);
    GP_COMMAD_PA(2); SPI_WriteData(0x0F);SPI_WriteData(0x00);  //(0x0F,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x10);SPI_WriteData(0xF9);  //(0x10,0xF9);
    GP_COMMAD_PA(2); SPI_WriteData(0x11);SPI_WriteData(0xFB);  //(0x11,0xFB);
    GP_COMMAD_PA(2); SPI_WriteData(0x12);SPI_WriteData(0x87);  //(0x12,0x87);
    GP_COMMAD_PA(2); SPI_WriteData(0x13);SPI_WriteData(0xEF);  //(0x13,0xEF);
    GP_COMMAD_PA(2); SPI_WriteData(0x14);SPI_WriteData(0x8B);  //(0x14,0x8B);
    GP_COMMAD_PA(2); SPI_WriteData(0x15);SPI_WriteData(0xEE);  //(0x15,0xEE);
    GP_COMMAD_PA(2); SPI_WriteData(0x16);SPI_WriteData(0xC5);  //(0x16,0xC5);
    GP_COMMAD_PA(2); SPI_WriteData(0x17);SPI_WriteData(0x81);  //(0x17,0x81);
    GP_COMMAD_PA(2); SPI_WriteData(0x18);SPI_WriteData(0x87);  //(0x18,0x87);
    GP_COMMAD_PA(2); SPI_WriteData(0x19);SPI_WriteData(0xB5);  //(0x19,0xB5);
    GP_COMMAD_PA(2); SPI_WriteData(0x1A);SPI_WriteData(0x71);  //(0x1A,0x71);
    GP_COMMAD_PA(2); SPI_WriteData(0x1B);SPI_WriteData(0x0E);  //(0x1B,0x0E);
    GP_COMMAD_PA(2); SPI_WriteData(0x1C);SPI_WriteData(0xFF);  //(0x1C,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x1D);SPI_WriteData(0xFF);  //(0x1D,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x1E);SPI_WriteData(0xFF);  //(0x1E,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x1F);SPI_WriteData(0xFF);  //(0x1F,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x20);SPI_WriteData(0xFF);  //(0x20,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x21);SPI_WriteData(0xFF);  //(0x21,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x22);SPI_WriteData(0xFF);  //(0x22,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x23);SPI_WriteData(0xFF);  //(0x23,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x24);SPI_WriteData(0xFF);  //(0x24,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x25);SPI_WriteData(0xFF);  //(0x25,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x26);SPI_WriteData(0xFF);  //(0x26,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x27);SPI_WriteData(0x1F);  //(0x27,0x1F);
    GP_COMMAD_PA(2); SPI_WriteData(0x28);SPI_WriteData(0xFF);  //(0x28,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x29);SPI_WriteData(0xFF);  //(0x29,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x2A);SPI_WriteData(0xFF);  //(0x2A,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x2B);SPI_WriteData(0xFF);  //(0x2B,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x2C);SPI_WriteData(0xFF);  //(0x2C,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x2D);SPI_WriteData(0x07);  //(0x2D,0x07);
    GP_COMMAD_PA(2); SPI_WriteData(0x33);SPI_WriteData(0x3F);  //(0x33,0x3F);
    GP_COMMAD_PA(2); SPI_WriteData(0x35);SPI_WriteData(0x7F);  //(0x35,0x7F);
    GP_COMMAD_PA(2); SPI_WriteData(0x36);SPI_WriteData(0x3F);  //(0x36,0x3F);
    GP_COMMAD_PA(2); SPI_WriteData(0x38);SPI_WriteData(0xFF);  //(0x38,0xFF);
    GP_COMMAD_PA(2); SPI_WriteData(0x3A);SPI_WriteData(0x80);  //(0x3A,0x80);
    GP_COMMAD_PA(2); SPI_WriteData(0x3B);SPI_WriteData(0x01);  //(0x3B,0x01);
    GP_COMMAD_PA(2); SPI_WriteData(0x3C);SPI_WriteData(0x80);  //(0x3C,0x80);
    GP_COMMAD_PA(2); SPI_WriteData(0x3D);SPI_WriteData(0x2C);  //(0x3D,0x2C);
    GP_COMMAD_PA(2); SPI_WriteData(0x3E);SPI_WriteData(0x00);  //(0x3E,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x3F);SPI_WriteData(0x90);  //(0x3F,0x90);
    GP_COMMAD_PA(2); SPI_WriteData(0x40);SPI_WriteData(0x05);  //(0x40,0x05);
    GP_COMMAD_PA(2); SPI_WriteData(0x41);SPI_WriteData(0x00);  //(0x41,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x42);SPI_WriteData(0xB2);  //(0x42,0xB2);
    GP_COMMAD_PA(2); SPI_WriteData(0x43);SPI_WriteData(0x00);  //(0x43,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x44);SPI_WriteData(0x40);  //(0x44,0x40);
    GP_COMMAD_PA(2); SPI_WriteData(0x45);SPI_WriteData(0x06);  //(0x45,0x06);
    GP_COMMAD_PA(2); SPI_WriteData(0x46);SPI_WriteData(0x00);  //(0x46,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x47);SPI_WriteData(0x00);  //(0x47,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x48);SPI_WriteData(0x9B);  //(0x48,0x9B);
    GP_COMMAD_PA(2); SPI_WriteData(0x49);SPI_WriteData(0xD2);  //(0x49,0xD2);
    GP_COMMAD_PA(2); SPI_WriteData(0x4A);SPI_WriteData(0x21);  //(0x4A,0x21);
    GP_COMMAD_PA(2); SPI_WriteData(0x4B);SPI_WriteData(0x43);  //(0x4B,0x43);
    GP_COMMAD_PA(2); SPI_WriteData(0x4C);SPI_WriteData(0x16);  //(0x4C,0x16);
    GP_COMMAD_PA(2); SPI_WriteData(0x4D);SPI_WriteData(0xC0);  //(0x4D,0xC0);
    GP_COMMAD_PA(2); SPI_WriteData(0x4E);SPI_WriteData(0x0F);  //(0x4E,0x0F);
    GP_COMMAD_PA(2); SPI_WriteData(0x4F);SPI_WriteData(0xF1);  //(0x4F,0xF1);
    GP_COMMAD_PA(2); SPI_WriteData(0x50);SPI_WriteData(0x78);  //(0x50,0x78);
    GP_COMMAD_PA(2); SPI_WriteData(0x51);SPI_WriteData(0x7A);  //(0x51,0x7A);
    GP_COMMAD_PA(2); SPI_WriteData(0x52);SPI_WriteData(0x34);  //(0x52,0x34);
    GP_COMMAD_PA(2); SPI_WriteData(0x53);SPI_WriteData(0x99);  //(0x53,0x99);
    GP_COMMAD_PA(2); SPI_WriteData(0x54);SPI_WriteData(0xA2);  //(0x54,0xA2);
    GP_COMMAD_PA(2); SPI_WriteData(0x55);SPI_WriteData(0x02);  //(0x55,0x02);
    GP_COMMAD_PA(2); SPI_WriteData(0x56);SPI_WriteData(0x14);  //(0x56,0x14);
    GP_COMMAD_PA(2); SPI_WriteData(0x57);SPI_WriteData(0xB8);  //(0x57,0xB8);
    GP_COMMAD_PA(2); SPI_WriteData(0x58);SPI_WriteData(0xDC);  //(0x58,0xDC);
    GP_COMMAD_PA(2); SPI_WriteData(0x59);SPI_WriteData(0xD4);  //(0x59,0xD4);
    GP_COMMAD_PA(2); SPI_WriteData(0x5A);SPI_WriteData(0xEF);  //(0x5A,0xEF);
    GP_COMMAD_PA(2); SPI_WriteData(0x5B);SPI_WriteData(0xF7);  //(0x5B,0xF7);
    GP_COMMAD_PA(2); SPI_WriteData(0x5C);SPI_WriteData(0xFB);  //(0x5C,0xFB);
    GP_COMMAD_PA(2); SPI_WriteData(0x5D);SPI_WriteData(0xFD);  //(0x5D,0xFD);
    GP_COMMAD_PA(2); SPI_WriteData(0x5E);SPI_WriteData(0x7E);  //(0x5E,0x7E);
    GP_COMMAD_PA(2); SPI_WriteData(0x5F);SPI_WriteData(0xBF);  //(0x5F,0xBF);
    GP_COMMAD_PA(2); SPI_WriteData(0x60);SPI_WriteData(0xEF);  //(0x60,0xEF);
    GP_COMMAD_PA(2); SPI_WriteData(0x61);SPI_WriteData(0xE6);  //(0x61,0xE6);
    GP_COMMAD_PA(2); SPI_WriteData(0x62);SPI_WriteData(0x76);  //(0x62,0x76);
    GP_COMMAD_PA(2); SPI_WriteData(0x63);SPI_WriteData(0x73);  //(0x63,0x73);
    GP_COMMAD_PA(2); SPI_WriteData(0x64);SPI_WriteData(0xBB);  //(0x64,0xBB);
    GP_COMMAD_PA(2); SPI_WriteData(0x65);SPI_WriteData(0xDD);  //(0x65,0xDD);
    GP_COMMAD_PA(2); SPI_WriteData(0x66);SPI_WriteData(0x6E);  //(0x66,0x6E);
    GP_COMMAD_PA(2); SPI_WriteData(0x67);SPI_WriteData(0x37);  //(0x67,0x37);
    GP_COMMAD_PA(2); SPI_WriteData(0x68);SPI_WriteData(0x8C);  //(0x68,0x8C);
    GP_COMMAD_PA(2); SPI_WriteData(0x69);SPI_WriteData(0x08);  //(0x69,0x08);
    GP_COMMAD_PA(2); SPI_WriteData(0x6A);SPI_WriteData(0x31);  //(0x6A,0x31);
    GP_COMMAD_PA(2); SPI_WriteData(0x6B);SPI_WriteData(0xB8);  //(0x6B,0xB8);
    GP_COMMAD_PA(2); SPI_WriteData(0x6C);SPI_WriteData(0xB8);  //(0x6C,0xB8);
    GP_COMMAD_PA(2); SPI_WriteData(0x6D);SPI_WriteData(0xB8);  //(0x6D,0xB8);
    GP_COMMAD_PA(2); SPI_WriteData(0x6E);SPI_WriteData(0xB8);  //(0x6E,0xB8);
    GP_COMMAD_PA(2); SPI_WriteData(0x6F);SPI_WriteData(0xB8);  //(0x6F,0xB8);
    GP_COMMAD_PA(2); SPI_WriteData(0x70);SPI_WriteData(0x5C);  //(0x70,0x5C);
    GP_COMMAD_PA(2); SPI_WriteData(0x71);SPI_WriteData(0x2E);  //(0x71,0x2E);
    GP_COMMAD_PA(2); SPI_WriteData(0x72);SPI_WriteData(0x17);  //(0x72,0x17);
    GP_COMMAD_PA(2); SPI_WriteData(0x73);SPI_WriteData(0x00);  //(0x73,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x74);SPI_WriteData(0x00);  //(0x74,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x75);SPI_WriteData(0x00);  //(0x75,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x76);SPI_WriteData(0x00);  //(0x76,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x77);SPI_WriteData(0x00);  //(0x77,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x78);SPI_WriteData(0x00);  //(0x78,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x79);SPI_WriteData(0x00);  //(0x79,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x7A);SPI_WriteData(0xDC);  //(0x7A,0xDC);
    GP_COMMAD_PA(2); SPI_WriteData(0x7B);SPI_WriteData(0xDC);  //(0x7B,0xDC);
    GP_COMMAD_PA(2); SPI_WriteData(0x7C);SPI_WriteData(0xDC);  //(0x7C,0xDC);
    GP_COMMAD_PA(2); SPI_WriteData(0x7D);SPI_WriteData(0xDC);  //(0x7D,0xDC);
    GP_COMMAD_PA(2); SPI_WriteData(0x7E);SPI_WriteData(0xDC);  //(0x7E,0xDC);
    GP_COMMAD_PA(2); SPI_WriteData(0x7F);SPI_WriteData(0x6E);  //(0x7F,0x6E);
    GP_COMMAD_PA(2); SPI_WriteData(0x0B);SPI_WriteData(0x00);  //(0x0B,0x00);

    GP_COMMAD_PA(2); SPI_WriteData(0xB1);SPI_WriteData(0x03);  //(0xB1,0x03);
    GP_COMMAD_PA(2); SPI_WriteData(0x2C);SPI_WriteData(0x2C);  //(0x2C,0x2C);

    GP_COMMAD_PA(2); SPI_WriteData(0xB1);SPI_WriteData(0x00);  //(0xB1,0x00);
    GP_COMMAD_PA(2); SPI_WriteData(0x89);SPI_WriteData(0x03);  //(0x89,0x03);

    GP_COMMAD_PA(1);
    SPI_WriteData(0x11);
    delay(120);
    GP_COMMAD_PA(1);
    SPI_WriteData(0x29);
    delay(120);
}

void SSD2828::Initial(void) {
    SPI_WriteCmd(CONFIGURATION_REGISTER);
    SPI_WriteData(0x50);  // 50=TX_CLK 70=PCLK
    SPI_WriteData(0x00);  // Configuration Register

    SPI_WriteCmd(VC_CONTROL_REGISTER);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);  // VC(Virtual ChannelID) Control Register

    SPI_WriteCmd(PLL_CONTROL_REGISTER);
    SPI_WriteData(0x00);  // 1=PLL disable
    SPI_WriteData(0x00);

    SPI_WriteCmd(PLL_CONFIGURATION_REGISTER);  // PLL=(TX_CLK/MS)*NS
    SPI_WriteData(0x14);  // 14,D7-0=NS(0x01 : NS=1)
    SPI_WriteData(0x42);  // 42,D15-14=PLL00=62.5-125 01=126-250 10=251-500 11=501-1000  DB12-8=MS(01:MS=1)

    SPI_WriteCmd(CLOCK_CONTROL_REGISTER);  // LP Clock Divider LP clock = 400MHz / LPD / 8 = 480 / 8/ 8 = 7MHz
    SPI_WriteData(0x03);  // D5-0=LPD=0x1 ®C Divide by 2
    SPI_WriteData(0x00);

    SPI_WriteCmd(PLL_CONTROL_REGISTER);
    SPI_WriteData(0x01);  // 1=PLL disable
    SPI_WriteData(0x00);

    SPI_WriteCmd(LANE_CONFIGURATION_REGISTER);  // MIPI lane configuration
    SPI_WriteData(0x00);  // 11=4LANE 10=3LANE 01=2LANE 00=1LANE
    SPI_WriteData(0x00);

    SPI_WriteCmd(DELAY_ADJUSTMENT_REGISTER_1);
    SPI_WriteData(0x02);
    SPI_WriteData(0x23);  // p1: HS-Data-zero  p2: HS-Data- prepare  --> 8031 issue
    // delay_ms(100);

    // LCD_initial();
    SPI_WriteCmd(CONFIGURATION_REGISTER);
    SPI_WriteData(0x50);
    SPI_WriteData(0x00);  // Configuration Register

    SPI_WriteCmd(VC_CONTROL_REGISTER);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);  // VC(Virtual ChannelID) Control Register

    SPI_WriteCmd(PLL_CONTROL_REGISTER);
    SPI_WriteData(0x00);  // 1=PLL disable
    SPI_WriteData(0x00);

    SPI_WriteCmd(PLL_CONFIGURATION_REGISTER);  //
    SPI_WriteData(0x2d);  // 14,D7-0=NS(0x01 : NS=1)         //0x25
    SPI_WriteData(0x82);  // 42,D15-14=PLL00=62.5-125 01=126-250 10=251-500 11=501-1000  DB12-8=MS(01:MS=1)

    SPI_WriteCmd(CLOCK_CONTROL_REGISTER);  // LP Clock Divider LP clock = 400MHz / LPD / 8 = 480 / 8/ 8 = 7MHz
    SPI_WriteData(0x07);  // D5-0=LPD=0x1 ®C Divide by 2
    SPI_WriteData(0x00);

    SPI_WriteCmd(PLL_CONTROL_REGISTER);
    SPI_WriteData(0x01);  // 1=PLL disable
    SPI_WriteData(0x00);

    SPI_WriteCmd(DELAY_ADJUSTMENT_REGISTER_1);
    SPI_WriteData(0x02);
    SPI_WriteData(0x23);  // p1: HS-Data-zero  p2: HS-Data- prepare  --> 8031 issue
    delay(100);

    SPI_WriteCmd(0xCA);
    SPI_WriteData(0x01);  // CLK Prepare
    SPI_WriteData(0x23);  // Clk Zero

    SPI_WriteCmd(0xCB);  // local_write_reg(addr=0xCB,data=0x0510)
    SPI_WriteData(0x10);  // Clk Post
    SPI_WriteData(0x05);  // Clk Per

    SPI_WriteCmd(0xCC);  // local_write_reg(addr=0xCC,data=0x100A)
    SPI_WriteData(0x05);  // HS Trail
    SPI_WriteData(0x10);  // Clk Trail

    SPI_WriteCmd(0xD0);
    SPI_WriteData(0x00);
    SPI_WriteData(0x00);

    /* //RGB interface configuration
    SPI_WriteCmd(RGB_INTERFACE_CONTROL_REGISTER_1);
    SPI_WriteData(0x18); //HSPW 07
    SPI_WriteData(0x02); //VSPW 05

    SPI_WriteCmd(RGB_INTERFACE_CONTROL_REGISTER_2);
    SPI_WriteData(0xa0); //HBPD 0x64=100
    SPI_WriteData(0x0a); //VBPD 8 ºı–°œ¬“∆

    SPI_WriteCmd(RGB_INTERFACE_CONTROL_REGISTER_3);
    SPI_WriteData(0xa0); //HFPD 8
    SPI_WriteData(0x0c); //VFPD 10
    */

    SPI_WriteCmd(RGB_INTERFACE_CONTROL_REGISTER_1); //local_write_reg(addr=0xB2,data=0x1224)
    SPI_WriteData(LCD_HSPW); //HSA
    SPI_WriteData(LCD_VSPW); //VSA

    SPI_WriteCmd(RGB_INTERFACE_CONTROL_REGISTER_2); //local_write_reg(addr=0xB2,data=0x1224)
    SPI_WriteData(LCD_HBPD); //HBP
    SPI_WriteData(LCD_VBPD); //VBP

    SPI_WriteCmd(RGB_INTERFACE_CONTROL_REGISTER_3); //local_write_reg(addr=0xB3,data=0x060C)
    SPI_WriteData(LCD_HFPD); //HFP
    SPI_WriteData(LCD_VFPD); //VFP

    SPI_WriteCmd(RGB_INTERFACE_CONTROL_REGISTER_4); //Horizontal active period 400
    SPI_WriteData(0x90); //013F=319 02D0=72
    SPI_WriteData(0x01);

    SPI_WriteCmd(RGB_INTERFACE_CONTROL_REGISTER_5); //Vertical active period 1280
    SPI_WriteData(0x00); //
    SPI_WriteData(0x05);

    SPI_WriteCmd(RGB_INTERFACE_CONTROL_REGISTER_6); //RGB CLK 16BPP=00 18BPP=01
    SPI_WriteData(0x0b); //D7=0 D6=0 D5=0 D1-0=11 ®C 24bpp //07
    SPI_WriteData(0xc0); //D15=VS D14=HS D13=CLK D12-9=NC D8=0=Video with blanking packet. 00-F0

    // MIPI lane configuration
    SPI_WriteCmd(LANE_CONFIGURATION_REGISTER); //Õ®µ¿ ˝
    SPI_WriteData(0x03); //11=4LANE 10=3LANE 01=2LANE 00=1LANE
    SPI_WriteData(0x00);

    SPI_WriteCmd(PLL_LOCK_REGISTER); // 05=BGR 04=RGB
    SPI_WriteData(0x01); //D0=0=RGB 1:BGR D1=1=Most significant byte sent first
    SPI_WriteData(0x00);

    SPI_WriteCmd(ANALOG_CONTROL_REGISTER_4);
    SPI_WriteData(0x58);
    SPI_WriteData(0x00);

    SPI_WriteCmd(CONFIGURATION_REGISTER);
    SPI_WriteData(0x4B);
    SPI_WriteData(0x02);

    SPI_WriteCmd(0x2c);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


