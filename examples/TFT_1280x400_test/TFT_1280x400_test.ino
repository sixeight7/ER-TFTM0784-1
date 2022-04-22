///////////////////////////////////////////////////////////////////////////////
//
// ER-TFTM0784-1  7.84 INCH TFT LCD  1280*400  ( RA8876 + SSD2828 + GT911 )
//
// 2020-04-14 : Ported to Arduino by Craig Iannello
//              Original by Javen Liu (Original heading next section)
//
// The demo code is pretty much verbatim of Javen's original.
//
// Devices accessed and pin usage:
//
//    CHIP    PURPOSE           INTERFACE
//    RA8876  Graphics Driver   SPI 4-Wire OR 8/16 bit Parallel
//    GT911   Touch             I2C + Interrupt flag output
//    SSD2828 MIPI bridge       SPI 4-Wire
//
// Note the two seperate SPI's.  I read that the RA8876 doesn't properly
// tri-state (release) the bus while it's unselected. I was too lazy to test
// that, and just left the SSD2828 as software serial on some pins,
// and hooked the HW SPI up to the RA8876. The 2828 only needs to be spoken
// to on powerup and doesnt need to be fast.
//
// The touch panel code has been adapted for the i2c bus using Wire().
//
///////////////////////////////////////////////////////////////////////////////
//
// EASTRISING TECHNOLOGY CO,.LTD.
//
// Module    : ER-TFTM0784-1  7.84 INCH TFT LCD  1280*400
// Lanuage   : C++
// Create    : JAVEN LIU
// Date      : 2018-12-23
// Drive IC  : RA8876     FLASH:W25Q128FV  128M BIT   FONT CHIP: 30L24T3Y   CTP:GT911
// INTERFACE : 16BIT 8080      CTP:IIC    SSD2828: 3SPI
// MCU     : STC12LE5C60S2     1T MCU
// MCU VDD   : 3.3V
// MODULE VDD : 5V OR 3.3V
//
///////////////////////////////////////////////////////////////////////////////

#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
#include <i2c_t3.h>
#else
#include <Wire.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>
#include <ER-TFTM0784-1.h>
#include <2828.h>
#include <Goodix.h>

#include "demo.h"
#include "teensy40_front.h"
#include "GroteskBold16x32.h"
#include "GroteskBold24x48.h"
#include "GroteskBold32x64.h"
#include "symbols16x16.h"

//-----------------------------------------------------------------------------
// the following global variables are declared as externs in the library and
// are defined below.  Sorry, this is probably poor style.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Setup communications method for RA8876 display controller
//-----------------------------------------------------------------------------

// 1: RA8876: Parallel 16 bits 8080
/*#define RS_8876_PIN 33
  #define CS_PAR_8876_PIN 34
  #define WR_8876_PIN 35
  #define RD_8876_PIN 36
  #define NEXT_8876_PIN 37
  #define D0_PIN 38
  #define D1_PIN 39
  #define D2_PIN 40
  #define D3_PIN 41
  #define D4_PIN 42
  #define D5_PIN 43
  #define D6_PIN 44
  #define D7_PIN 45
  #define D8_PIN 46
  #define D9_PIN 47
  #define D10_PIN 48
  #define D11_PIN 49
  #define D12_PIN 50
  #define D13_PIN 51
  #define D14_PIN 52
  #define D15_PIN 53
  #define RST_PAR_PIN 14
  ER_TFTM0784 tft = ER_TFTM0784(RS_8876_PIN, CS_PAR_8876_PIN, WR_8876_PIN, RD_8876_PIN, NEXT_8876_PIN,
   D0_PIN, D1_PIN, D2_PIN, D3_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN, D8_PIN, D9_PIN, D10_PIN, D11_PIN,
   D12_PIN, D13_PIN, D14_PIN, D15_PIN, RST_PAR_PIN);*/

// 2: RA8876: SPI 4-Wire

#define SPI_SPEED 42000000

#define IS_VCTOUCH

#ifdef IS_VCTOUCH
// Pin numbers for VC-touch
#define SDI_PIN 11
#define SDO_PIN 12
#define SCLK_PIN 13
#define CS_PIN 10
#define RST_SER_PIN 9
ER_TFTM0784 tft = ER_TFTM0784(SPI_SPEED, SDI_PIN, SDO_PIN, SCLK_PIN, CS_PIN, RST_SER_PIN);

//-----------------------------------------------------------------------------
// SPI pins for the 2828 LCD panel driver (used in all modes)
//
// No attempt was made to share this SPI with the RA8876 SPI because I read
// that there are problems with the RA8876 not properly
// tri-stating (disconnecting) its bus pins when it is not selected.
//
// Note that this interface is only used once on powerup to setup the panel
// driver, so it doesnt need to be fast. It uses software (bit bang) SPI.
//-----------------------------------------------------------------------------

#define CS_2828_PIN 25
#define RST_2828_PIN 28
#define SDI_2828_PIN 26
#define SCLK_2828_PIN 27

SSD2828 Master_bridge = SSD2828(SDI_2828_PIN, SCLK_2828_PIN, CS_2828_PIN, RST_2828_PIN);

//-----------------------------------------------------------------------------
// I2C pins for GT911 touch controller (used in all modes)
//-----------------------------------------------------------------------------

//#define SDA_911_PIN    18
//#define SCL_911_PIN    19
#define INT_911_PIN    30
#define RST_911_PIN    29

#define BACKLIGHT_PIN 24

#else
// Pin numbers for experimental side board
#define SDI_PIN 11
#define SDO_PIN 12
#define SCLK_PIN 13
#define CS_PIN 15
#define RST_SER_PIN 14
ER_TFTM0784 tft = ER_TFTM0784(SPI_SPEED, SDI_PIN, SDO_PIN, SCLK_PIN, CS_PIN, RST_SER_PIN);

//-----------------------------------------------------------------------------
// SPI pins for the 2828 LCD panel driver (used in all modes)
//
// No attempt was made to share this SPI with the RA8876 SPI because I read
// that there are problems with the RA8876 not properly
// tri-stating (disconnecting) its bus pins when it is not selected.
//
// Note that this interface is only used once on powerup to setup the panel
// driver, so it doesnt need to be fast. It uses software (bit bang) SPI.
//-----------------------------------------------------------------------------

#define CS_2828_PIN 20
#define RST_2828_PIN 21
#define SDI_2828_PIN 22
#define SCLK_2828_PIN 23

SSD2828 Master_bridge = SSD2828(SDI_2828_PIN, SCLK_2828_PIN, CS_2828_PIN, RST_2828_PIN);

//-----------------------------------------------------------------------------
// I2C pins for GT911 touch controller (used in all modes)
//-----------------------------------------------------------------------------

//#define SDA_911_PIN    18
//#define SCL_911_PIN    19
#define INT_911_PIN    16
#define RST_911_PIN    17

#define BACKLIGHT_PIN 2
#endif

Goodix touch = Goodix(INT_911_PIN, RST_911_PIN);

//-----------------------------------------------------------------------------
// The demo code is pretty much verbatim of Javen's original.
//-----------------------------------------------------------------------------

void setup(void)
{
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);
  
  Serial.begin(9600);
  Serial.println("Starting in one second...");
  delay(1000);
  Serial.println("Starting.");

  Serial.println("SSD2828 reset");
  Master_bridge.Reset();

  Serial.println("RA8876  IO init");
  tft.RA8876_IO_Init();
  Serial.println("RA8876  HW reset");
  tft.RA8876_HW_Reset();

  Serial.println("SSD2828 init");
  Master_bridge.Initial();

  Serial.println("RA8876  init");
  tft.RA8876_initial();

  Serial.println("Display ON");
  Display_on();

  Serial.println("GT911   touch init");
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
#else
  Wire.setClock(400000);
  Wire.begin();
#endif
  delay(300);
  touch.setHandler(handleTouch);
  touchStart();
  Fillscreen();
  Clear_user_font_canvas();
  delay(5000);
  Fix_rinkydink_font(Symbols24x24);
}

//-----------------------------------------------------------------------------
void loop(void)
{

  // BackLight brightness control : RA8876's PWM0
  Serial.println("RA8876  init backlight pwm");
  tft.Backlight_ON();

  //Serial.println("Alpha blending");
  //App_Demo_Alpha_Blending();
  //Serial.println("TpTest");
  //Touch_Demo();
  Serial.println("Geometric");
  //Geometric();
  Serial.println("Text");
  //Text_Demo();
  Serial.println("Mono");
  //mono_Demo();
  Serial.println("Gray");
  //gray();
  delay(2000);
  Serial.println("DMA");
  //DMA_Demo();
  Serial.println("Pattern fill");
  //BTE_Pattern_Fill();
  Serial.println("Color expansion");
  //BTE_Color_Expansion();
  Serial.println("Waveform");
  //App_Demo_Waveform();
  Serial.println("Scroller");
  //App_Demo_Scrolling_Text();
  Serial.println("Slide frame buffer");
  //App_Demo_slide_frame_buffer();
  Serial.println("Multi frame buffer");
  //App_Demo_multi_frame_buffer();
  Serial.println("Show picture");
  Show_pic();
  Serial.println("User font");
  //Clear_user_font_canvas();
  Show_user_font(Grotesk16x32, "Test 123...PQ", 370, 70, White);
  Show_user_font(GroteskBold24x48, "Test 123...", 370, 120, White);
  Show_user_font(GroteskBold32x64, "Test 123...", 370, 200, Yellow);
  Show2_user_font(GroteskBold32x64, "Test 567...", 370, 200, Yellow);
  Show2_user_font(Symbols24x24,  " !\"#" , 370, 300, White);
}
// Touchimplementation
void handleTouch(int8_t contacts, GTPoint *points) {
  Serial.printf("Contacts: %d\n", contacts);
  uint16_t colours[5] = { color65k_red, color65k_green, color65k_blue, color65k_cyan, color65k_purple };
  for (uint8_t i = 0; i < contacts; i++) {
    uint16_t x = points[i].x_lsb + (points[i].x_msb << 8);
    uint16_t y = points[i].y_lsb + (points[i].y_msb << 8);
    uint16_t area = points[i].area_lsb + (points[i].area_msb << 8);
    Serial.println("ID:" + String(points[i].trackId) + ", X:" + String(x) + ", Y:" + String(y) + ", area:" + String(area));
    Draw_Big_Point(x, y, colours[i]);
  }
}

void touchStart() {
  if (touch.begin() != true) {
    Serial.println("! Module reset failed");
  } else {
    Serial.println("Module reset OK");
  }

  Serial.print("Check ACK on addr request on 0x");
  Serial.print(touch.i2cAddr, HEX);

  Wire.beginTransmission(touch.i2cAddr);
  int error = Wire.endTransmission();
  if (error == 0) {
    Serial.println(": SUCCESS");
  } else {
    Serial.print(": ERROR #");
    Serial.println(error);
  }
  //Serial.println("GT911 configuration version: " + touch.configurationVersion());
}

void Touch_Demo(void)
{
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(0);//Layer 1
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);

  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  tft.Foreground_color_65k(Black);
  tft.Background_color_65k(Blue2);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(0, 450);
  tft.Show_String("www.buydisplay.ocm  CTP test");

  tft.Goto_Text_XY(0, 1230);
  tft.Show_String("exit");
  tft.Goto_Text_XY(376, 1215);
  tft.Show_String("clear");

  delay(500);

#define DEMO_TIME 10000;
  uint32_t runtime = millis() + DEMO_TIME;

  while (runtime > millis())
  {
    touch.loop();
    //delay(1);
  }
}

void Draw_Big_Point(uint16_t x, uint16_t y, uint16_t colour)
{
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(colour);
  tft.Line_Start_XY(y - 2, x - 2);
  tft.Line_End_XY(y + 2, x + 2);
  tft.Start_Square_Fill();
}

void Fillscreen() {
  tft.Canvas_Image_Start_address(layer1_start_addr);//Layer 1
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);

  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
}

//-----------------------------------------------------------------------------
void NextStep(void)
{
  delay(500);
  /*
    while(next)
    {
      Delay100ms(1);
    }
    Delay100ms(10);
  */
}
//-----------------------------------------------------------------------------

void Display_on() {
  tft.Display_ON();
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(White);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
}

void Show_pic() {
  // 240x320
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);

  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(layer1_start_addr);//
  tft.Main_Image_Width(400);
  tft.Active_Window_XY(10, 10);
  tft.Active_Window_WH(320, 240);
  tft.Goto_Pixel_XY(10, 10);
  tft.Show_picture(240 * 320, teensy40_front);

  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);
  /*tft.Select_Main_Window_16bpp();
    tft.Main_Image_Start_Address(0);
    tft.Main_Image_Width(400);
    tft.Main_Window_Start_XY(0, 0);

    //switch to layer2 update screen, and execute color expansion and copy to layer1
    tft.Canvas_Image_Start_address(layer2_start_addr);//
    tft.Foreground_color_65k(Black);
    tft.Line_Start_XY(0, 0);
    tft.Line_End_XY(399, 1279);
    tft.Start_Square_Fill();
    tft.SFI_DMA_Source_Start_Address(0);//
    tft.Start_SFI_DMA();
    tft.Check_Busy_SFI_DMA();

    tft.BTE_S0_Color_16bpp();

    tft.BTE_S1_Color_16bpp();

    tft.BTE_Destination_Color_16bpp();
    tft.BTE_Destination_Memory_Start_Address(layer2_start_addr);
    tft.BTE_Destination_Image_Width(400);
    tft.BTE_Destination_Window_Start_XY(0 + 70, 200 + 70);
    tft.BTE_Window_Size(160, 160);
    tft.Foreground_color_65k(color65k_green);
    tft.Background_color_65k(color65k_black);
    tft.BTE_ROP_Code(15);
    tft.BTE_Operation_Code(8); //BTE color expansion

    tft.BTE_Enable();
    tft.LCD_CmdWrite(0x04);

    tft.Show_picture(10 * 160, f1);
    //tft.Show_picture(80 * 80, pic_80x80);
    tft.Check_Mem_WR_FIFO_Empty();
    tft.Check_BTE_Busy();

    tft.Foreground_color_65k(White);
    tft.Background_color_65k(Black);
    tft.CGROM_Select_Internal_CGROM();
    tft.Font_Select_12x24_24x24();
    tft.Goto_Text_XY(250, 450);
    tft.Show_String("Color Expansion");
    tft.Goto_Text_XY(274, 450);
    tft.Show_String("Color With chroma key");

    tft.Foreground_color_65k(color65k_blue);
    tft.Background_color_65k(color65k_red);

    //BTE memory(move) layer2 to layer1
    //BTE_S0_Color_16bpp();
    tft.BTE_S0_Memory_Start_Address(layer2_start_addr);
    tft.BTE_S0_Image_Width(400);
    tft.BTE_S0_Window_Start_XY(0, 0);

    //BTE_Destination_Color_16bpp();
    tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
    tft.BTE_Destination_Image_Width(400);
    tft.BTE_Destination_Window_Start_XY(0, 0);
    tft.BTE_Window_Size(400, 1280);

    tft.BTE_ROP_Code(12);
    tft.BTE_Operation_Code(2); //BTE move
    tft.BTE_Enable();      //memory copy s0(layer2) to layer1
    tft.Check_BTE_Busy();*/

  delay(2000);
}

void BTE_Color_Expansion(void)
{
  uint16_t i, j;
  //uint32_t im = 1;

  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  /*tft.Canvas_Image_Start_address(layer1_start_addr);//Layer 1
    tft.Canvas_image_width(400);//
    tft.Active_Window_XY(0, 0);
    tft.Active_Window_WH(400, 1280);

    tft.Foreground_color_65k(Black);
    tft.Line_Start_XY(0, 0);
    tft.Line_End_XY(399, 1279);
    tft.Start_Square_Fill();



    //DMA initial setting
    tft.Enable_SFlash_SPI();
    tft.Select_SFI_1();
    tft.Select_SFI_DMA_Mode();
    tft.Select_SFI_24bit_Address();

    //tft.Select_SFI_Waveform_Mode_0();
    tft.Select_SFI_Waveform_Mode_3();

    //tft.Select_SFI_0_DummyRead(); //normal read mode
    tft.Select_SFI_8_DummyRead(); //1byte dummy cycle
    //tft.Select_SFI_16_DummyRead();
    //tft.Select_SFI_24_DummyRead();

    tft.Select_SFI_Single_Mode();
    //tft.Select_SFI_Dual_Mode0();
    //tft.Select_SFI_Dual_Mode1();

    tft.SPI_Clock_Period(0);


    tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
    tft.SFI_DMA_Transfer_Width_Height(400, 1280);
    tft.SFI_DMA_Source_Width(400);//

    tft.SFI_DMA_Source_Start_Address(im * 400 * 1280 * 3); //
    tft.Start_SFI_DMA();
    tft.Check_Busy_SFI_DMA(); */

  //color expansion and bte memory copy(move)

  j = 0;
  do
  {
    for (i = 0; i < 3; i++)
    {
      //switch to layer2 update screen, and execute color expansion and copy to layer1
      tft.Canvas_Image_Start_address(layer2_start_addr);//
      tft.Foreground_color_65k(Black);
      tft.Line_Start_XY(0, 0);
      tft.Line_End_XY(399, 1279);
      tft.Start_Square_Fill();
      tft.SFI_DMA_Source_Start_Address(0);//
      tft.Start_SFI_DMA();
      tft.Check_Busy_SFI_DMA();

      tft.BTE_S0_Color_16bpp();

      tft.BTE_S1_Color_16bpp();

      tft.BTE_Destination_Color_16bpp();
      tft.BTE_Destination_Memory_Start_Address(layer2_start_addr);
      tft.BTE_Destination_Image_Width(400);
      tft.BTE_Destination_Window_Start_XY(0 + 70, 0 + 70);
      tft.BTE_Window_Size(160, 160);
      tft.Foreground_color_65k(color65k_blue);
      tft.Background_color_65k(color65k_red);
      tft.BTE_ROP_Code(15);
      tft.BTE_Operation_Code(8); //BTE color expansion

      tft.BTE_Enable();
      tft.LCD_CmdWrite(0x04);

      switch (i)
      {
        case 0 :
          tft.Show_picture(10 * 160, f1);
          tft.Check_Mem_WR_FIFO_Empty();
          tft.Check_BTE_Busy();
          break;
        case 1 :
          tft.Show_picture(10 * 160, f2);
          tft.Check_Mem_WR_FIFO_Empty();
          tft.Check_BTE_Busy();
          break;
        case 2 :
          tft.Show_picture(10 * 160, f3);
          tft.Check_Mem_WR_FIFO_Empty();
          tft.Check_BTE_Busy();
          break;
        default:
          break;
      }

      tft.Foreground_color_65k(White);
      tft.Background_color_65k(Black);
      tft.CGROM_Select_Internal_CGROM();
      tft.Font_Select_12x24_24x24();
      tft.Goto_Text_XY(274, 70);
      tft.Show_String("Color Expansion");



      tft.Foreground_color_65k(color65k_blue);
      tft.Background_color_65k(color65k_red);

      tft.BTE_Operation_Code(9); //BTE color expansion with chroma key
      tft.BTE_Destination_Window_Start_XY(70, 460);
      tft.BTE_Enable();
      tft.LCD_CmdWrite(0x04);

      switch (i)
      {
        case 0 :
          tft.Show_picture(10 * 160, f1);
          tft.Check_Mem_WR_FIFO_Empty();
          tft.Check_BTE_Busy();
          break;
        case 1 :
          tft.Show_picture(10 * 160, f2);
          tft.Check_Mem_WR_FIFO_Empty();
          tft.Check_BTE_Busy();
          break;
        case 2 :
          tft.Show_picture(10 * 160, f3);
          tft.Check_Mem_WR_FIFO_Empty();
          tft.Check_BTE_Busy();
          break;
        default:
          break;
      }


      tft.Foreground_color_65k(White);
      tft.Background_color_65k(Black);
      tft.CGROM_Select_Internal_CGROM();
      tft.Font_Select_12x24_24x24();
      tft.Goto_Text_XY(250, 450);
      tft.Show_String("Color Expansion");
      tft.Goto_Text_XY(274, 450);
      tft.Show_String("Color With chroma key");

      tft.Foreground_color_65k(color65k_blue);
      tft.Background_color_65k(color65k_red);

      //BTE memory(move) layer2 to layer1
      //BTE_S0_Color_16bpp();
      tft.BTE_S0_Memory_Start_Address(layer2_start_addr);
      tft.BTE_S0_Image_Width(400);
      tft.BTE_S0_Window_Start_XY(0, 0);

      //BTE_Destination_Color_16bpp();
      tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
      tft.BTE_Destination_Image_Width(400);
      tft.BTE_Destination_Window_Start_XY(0, 0);
      tft.BTE_Window_Size(400, 1280);

      tft.BTE_ROP_Code(12);
      tft.BTE_Operation_Code(2); //BTE move
      tft.BTE_Enable();      //memory copy s0(layer2) to layer1
      tft.Check_BTE_Busy();


      j++;
    }

  } while (j < 8);

  NextStep();

}
//-----------------------------------------------------------------------------
void BTE_Pattern_Fill(void)
{ uint32_t im = 1;

  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(0);//Layer 1
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);

  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();



  tft.Foreground_color_65k(White);
  tft.Background_color_65k(Blue2);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(0, 800);
  tft.Show_String("Demo BTE Pattern Fill");


  //DMA initial setting
  tft.Enable_SFlash_SPI();
  tft.Select_SFI_1();
  tft.Select_SFI_DMA_Mode();
  tft.Select_SFI_24bit_Address();

  //Select_SFI_Waveform_Mode_0();
  tft.Select_SFI_Waveform_Mode_3();

  //Select_SFI_0_DummyRead(); //normal read mode
  tft.Select_SFI_8_DummyRead(); //1byte dummy cycle
  //Select_SFI_16_DummyRead();
  //Select_SFI_24_DummyRead();

  tft.Select_SFI_Single_Mode();
  //Select_SFI_Dual_Mode0();
  //Select_SFI_Dual_Mode1();

  tft.SPI_Clock_Period(0);


  tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
  tft.SFI_DMA_Transfer_Width_Height(400, 1280);
  tft.SFI_DMA_Source_Width(400);//

  tft.SFI_DMA_Source_Start_Address(im * 400 * 1280 * 2 * 1); //
  tft.Start_SFI_DMA();
  tft.Check_Busy_SFI_DMA();


  //write 16x16 pattern to sdram
  tft.Pattern_Format_16X16();
  tft.Canvas_Image_Start_address(layer6_start_addr);//any layer
  tft.Canvas_image_width(16);
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(16, 16);
  tft.Goto_Pixel_XY(0, 0);
  tft.Show_picture(16 * 16, pattern16x16_16bpp);

  tft.Canvas_Image_Start_address(layer1_start_addr);//
  tft.Canvas_image_width(400);
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);


  tft.BTE_S0_Color_16bpp();
  tft.BTE_S0_Memory_Start_Address(layer6_start_addr);
  tft.BTE_S0_Image_Width(16);

  tft.BTE_S1_Color_16bpp();
  tft.BTE_S1_Memory_Start_Address(0);
  tft.BTE_S1_Image_Width(400);

  tft.BTE_Destination_Color_16bpp();
  tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
  tft.BTE_Destination_Image_Width(400);

  tft.BTE_ROP_Code(0xc);
  tft.BTE_Operation_Code(0x06);//pattern fill

  tft.BTE_S1_Window_Start_XY(0, 0);
  tft.BTE_Destination_Window_Start_XY(0, 40);
  tft.BTE_Window_Size(300, 300);

  tft.BTE_Enable();
  tft.Check_BTE_Busy();

  tft.Foreground_color_65k(Black);
  tft.Background_color_65k(White);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(330, 100);
  tft.Show_String("Pattern Fill");

  tft.Background_color_65k(color65k_red);
  tft.BTE_S1_Window_Start_XY(0, 0);
  tft.BTE_Destination_Window_Start_XY(0, 440);
  tft.BTE_Window_Size(300, 300);
  tft.BTE_Operation_Code(0x07);//pattern fill with chroma key

  tft.BTE_Enable();
  tft.Check_BTE_Busy();

  tft.Foreground_color_65k(Black);
  tft.Background_color_65k(White);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(330, 450);
  tft.Show_String("Pattern Fill With");

  tft.Foreground_color_65k(Black);
  tft.Background_color_65k(White);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(354, 450);
  tft.Show_String("Chroma Key");
  delay(1000);
  NextStep();
}
//-----------------------------------------------------------------------------
void App_Demo_Waveform(void)
{
  uint16_t i, h = 0;

  uint16_t point1y, point2y;
  uint16_t point21y, point22y;
  uint16_t point31y, point32y;
  point2y = 0; //initial value
  point22y = 0; //initial value
  point32y = 0; //initial value

#define grid_width 400
#define grid_high  1280
#define grid_gap 50

  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(0);//Layer 1
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);

  tft.Foreground_color_65k(Blue);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1247);
  tft.Start_Square_Fill();

  tft.Foreground_color_65k(Blue2);
  tft.Line_Start_XY(0, 1248);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  do {

    tft.Canvas_Image_Start_address(layer2_start_addr);//Layer 2


    tft.Foreground_color_65k(Black);  //clear layer2 to color black
    tft.Line_Start_XY(0, 0);
    tft.Line_End_XY(399, 1279);
    tft.Start_Square_Fill();




    for (i = 0; i <= grid_width; i += grid_gap)
    {
      tft.Foreground_color_65k(color65k_grayscale12);
      tft.Line_Start_XY(i, 0);
      tft.Line_End_XY(i, grid_high - 1);
      tft.Start_Line();
    }

    for (i = 0; i <= grid_high; i += grid_gap)
    {
      tft.Foreground_color_65k(color65k_grayscale12);
      tft.Line_Start_XY(0, i);
      tft.Line_End_XY(grid_width - 1, i);
      tft.Start_Line();
    }

    tft.Foreground_color_65k(Red);
    tft.Background_color_65k(Black);
    tft.CGROM_Select_Internal_CGROM();
    tft.Font_Select_12x24_24x24();
    tft.Goto_Text_XY(0, 0);
    tft.Show_String("Application Demo Waveform    www.buydisplay.com");


    //BTE memory(move) grid to layer1
    tft.BTE_S0_Color_16bpp();
    tft.BTE_S0_Memory_Start_Address(layer2_start_addr);
    tft.BTE_S0_Image_Width(400);
    tft.BTE_S0_Window_Start_XY(0, 0);


    tft.BTE_Destination_Color_16bpp();
    tft.BTE_Destination_Memory_Start_Address(0);
    tft.BTE_Destination_Image_Width(400);
    tft.BTE_Destination_Window_Start_XY(0, 0);
    tft.BTE_Window_Size(400, 1280);

    //move with ROP 0
    tft.BTE_ROP_Code(12); //memory copy s0(layer2)grid to layer1
    tft.BTE_Operation_Code(2); //BTE move
    tft.BTE_Enable();
    tft.Check_BTE_Busy();


    tft.Canvas_Image_Start_address(0);//Layer 1



    for (i = 0; i < 1280; i += 2)
    {
      // copy layer2 grid column to layer1
      tft.BTE_S0_Window_Start_XY(i, 0);

      tft.BTE_Destination_Window_Start_XY(0 + i, 0);
      tft.BTE_Window_Size(0, 400);
      tft.BTE_Enable();
      tft.Check_BTE_Busy();


      point1y = point2y;
      point2y = rand() % 90; //

      point21y = point22y;
      point22y = rand() % 99; //
      //
      point31y = point32y;
      point32y = rand() % 67; //

      tft.Foreground_color_65k(color65k_yellow);//
      //     Line_Start_XY(i+10,point1y+80);
      //     Line_End_XY(i+1+10,point2y+80);
      tft.Line_Start_XY(point1y + 80, i + 10);
      tft.Line_End_XY(point2y + 80, i + 1 + 10);
      tft.Start_Line();

      tft.Foreground_color_65k(color65k_purple);//
      //     Line_Start_XY(i+10,point21y+200);
      //     Line_End_XY(i+1+10,point22y+200);
      tft.Line_Start_XY(point21y + 200, i + 10);
      tft.Line_End_XY(point22y + 200, i + 1 + 10);
      tft.Start_Line();
      //
      tft.Foreground_color_65k(color65k_green);//
      //     Line_Start_XY(i+10,point31y+300);
      //     Line_End_XY(i+1+10,point32y+300);

      tft.Line_Start_XY(point31y + 300, i + 10);
      tft.Line_End_XY(point32y + 300, i + 1 + 10);
      tft.Start_Line();


      //delay(1000);
    }


    h++;
  }
  while (h < 7);

  NextStep();

}
//-----------------------------------------------------------------------------
void App_Demo_Scrolling_Text(void)
{
  uint16_t i;



  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(layer2_start_addr);
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);

  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();



  tft.Foreground_color_65k(White);
  tft.Background_color_65k(Blue2);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(0, Line52);
  tft.Show_String("Demo Scrolling Text");
  tft.Foreground_color_65k(White);
  tft.Background_color_65k(Black);
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(0, Line51);
  tft.Show_String("Used Move BTE with Chroma Key ");


  //DMA initial setting
  tft.Enable_SFlash_SPI();
  tft.Select_SFI_1();
  tft.Select_SFI_DMA_Mode();
  tft.Select_SFI_24bit_Address();

  //Select_SFI_Waveform_Mode_0();
  tft.Select_SFI_Waveform_Mode_3();

  //Select_SFI_0_DummyRead(); //normal read mode
  tft.Select_SFI_8_DummyRead(); //1byte dummy cycle
  //Select_SFI_16_DummyRead();
  //Select_SFI_24_DummyRead();

  tft.Select_SFI_Single_Mode();
  //Select_SFI_Dual_Mode0();
  //Select_SFI_Dual_Mode1();

  tft.SPI_Clock_Period(0);


  tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
  tft.SFI_DMA_Transfer_Width_Height(400, 1280);
  tft.SFI_DMA_Source_Width(400);//
  tft.SFI_DMA_Source_Start_Address(0);//
  tft.Start_SFI_DMA();
  tft.Check_Busy_SFI_DMA();


  //BTE memory(move) layer2 to layer1
  tft.BTE_S0_Color_16bpp();
  tft.BTE_S0_Memory_Start_Address(layer2_start_addr);
  tft.BTE_S0_Image_Width(400);
  tft.BTE_S0_Window_Start_XY(0, 0);



  tft.BTE_Destination_Color_16bpp();
  tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
  tft.BTE_Destination_Image_Width(400);
  tft.BTE_Destination_Window_Start_XY(0, 0);
  tft.BTE_Window_Size(400, 1280);

  tft.BTE_ROP_Code(12);
  tft.BTE_Operation_Code(2); //BTE move
  tft.BTE_Enable();      //memory copy s0(layer3) to layer1
  tft.Check_BTE_Busy();


  //write text to layer3
  tft.Canvas_Image_Start_address(layer3_start_addr);
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);

  tft.Foreground_color_65k(Red);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  tft.Foreground_color_65k(Green);
  tft.Background_color_65k(Red);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(24, 10);
  tft.Show_String("Demo Scrolling Text");

  tft.Foreground_color_65k(Yellow);
  tft.Background_color_65k(Red);
  tft.Goto_Text_XY(48, 10);
  tft.Show_String("Demo Scrolling Text");
  tft.Foreground_color_65k(Magenta);
  tft.Background_color_65k(Red);
  tft.Goto_Text_XY(72, 10);
  tft.Show_String("Demo Scrolling Text");
  tft.Foreground_color_65k(Grey);
  tft.Background_color_65k(Red);
  tft.Goto_Text_XY(96, 10);
  tft.Show_String("Demo Scrolling Text");


  //Move BTE with chroma key layer3 to layer2 then move layer2 to layer1 to display

  for (i = 0; i < 200; i += 8)
  {
    tft.Canvas_Image_Start_address(layer2_start_addr);
    tft.Canvas_image_width(400);//
    tft.Active_Window_XY(0, 0);
    tft.Active_Window_WH(400, 1280);

    tft.Foreground_color_65k(Black);
    tft.Line_Start_XY(0, 0);
    tft.Line_End_XY(399, 1279);
    tft.Start_Square_Fill();

    tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
    tft.SFI_DMA_Transfer_Width_Height(400, 1280);
    tft.SFI_DMA_Source_Width(400);//
    tft.SFI_DMA_Source_Start_Address(0);//
    tft.Start_SFI_DMA();
    tft.Check_Busy_SFI_DMA();

    //BTE memory(move) layer3 to layer2
    //BTE_S0_Color_16bpp();
    tft.BTE_S0_Memory_Start_Address(layer3_start_addr);
    tft.BTE_S0_Image_Width(400);
    tft.BTE_S0_Window_Start_XY(0, 0);



    //BTE_Destination_Color_16bpp();
    tft.BTE_Destination_Memory_Start_Address(layer2_start_addr);
    tft.BTE_Destination_Image_Width(400);
    tft.BTE_Destination_Window_Start_XY(391 - i, 72);
    tft.BTE_Window_Size(0 + i, 24 * 4);

    tft.Background_color_65k(Red);//

    tft.BTE_ROP_Code(12);
    tft.BTE_Operation_Code(5); //BTE move with chroma key
    tft.BTE_Enable();      //memory copy s0(layer3) to layer1
    tft.Check_BTE_Busy();




    //BTE memory(move) layer2 to layer1
    //BTE_S0_Color_16bpp();
    tft.BTE_S0_Memory_Start_Address(layer2_start_addr);
    tft.BTE_S0_Image_Width(400);
    tft.BTE_S0_Window_Start_XY(0, 0);

    //BTE_Destination_Color_16bpp();
    tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
    tft.BTE_Destination_Image_Width(400);
    tft.BTE_Destination_Window_Start_XY(0, 0);
    tft.BTE_Window_Size(400, 1280);

    tft.BTE_ROP_Code(12);
    tft.BTE_Operation_Code(2); //BTE move
    tft.BTE_Enable();      //memory copy s0(layer2) to layer1
    tft.Check_BTE_Busy();
  }

  for (i = 0; i < 100; i += 8)
  {
    tft.Canvas_Image_Start_address(layer2_start_addr);
    tft.Canvas_image_width(400);//
    tft.Active_Window_XY(0, 0);
    tft.Active_Window_WH(400, 1280);

    tft.Foreground_color_65k(Black);
    tft.Line_Start_XY(0, 0);
    tft.Line_End_XY(399, 1279);
    tft.Start_Square_Fill();

    tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
    tft.SFI_DMA_Transfer_Width_Height(400, 1280);
    tft.SFI_DMA_Source_Width(400);//
    tft.SFI_DMA_Source_Start_Address(layer4_start_addr);// ??
    tft.Start_SFI_DMA();
    tft.Check_Busy_SFI_DMA();

    //BTE memory(move) layer3 to layer2
    //BTE_S0_Color_16bpp();
    tft.BTE_S0_Memory_Start_Address(layer3_start_addr);
    tft.BTE_S0_Image_Width(400);
    tft.BTE_S0_Window_Start_XY(i, 0);



    //BTE_Destination_Color_16bpp();
    tft.BTE_Destination_Memory_Start_Address(layer2_start_addr);
    tft.BTE_Destination_Image_Width(400);
    tft.BTE_Destination_Window_Start_XY(0, 72);
    tft.BTE_Window_Size(399 - i, 24 * 4);

    tft.Background_color_65k(Red);//

    tft.BTE_ROP_Code(12);
    tft.BTE_Operation_Code(5); //BTE move with chroma key
    tft.BTE_Enable();          //memory copy s0(layer3) to layer1
    tft.Check_BTE_Busy();

    //     Main_Image_Width(800);
    //       Main_Window_Start_XY(0,0);
    //     Main_Image_Start_Address(layer2_start_addr);//switch display windows to
    //     delay(20);


    //BTE memory(move) layer2 to layer1
    //BTE_S0_Color_16bpp();
    tft.BTE_S0_Memory_Start_Address(layer2_start_addr);
    tft.BTE_S0_Image_Width(400);
    tft.BTE_S0_Window_Start_XY(0, 0);

    //BTE_Destination_Color_16bpp();
    tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
    tft.BTE_Destination_Image_Width(400);
    tft.BTE_Destination_Window_Start_XY(0, 0);
    tft.BTE_Window_Size(400, 1280);

    tft.BTE_ROP_Code(12);
    tft.BTE_Operation_Code(2); //BTE move
    tft.BTE_Enable();      //memory copy s0(layer2) to layer1
    tft.Check_BTE_Busy();
  }

  NextStep();

}
//-----------------------------------------------------------------------------
void App_Demo_multi_frame_buffer(void)
{ uint16_t i, j;
  uint32_t im = 1;
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  //DMA initial setting
  tft.Enable_SFlash_SPI();
  tft.Select_SFI_1();
  tft.Select_SFI_DMA_Mode();
  tft.Select_SFI_24bit_Address();

  //Select_SFI_Waveform_Mode_0();
  tft.Select_SFI_Waveform_Mode_3();

  //Select_SFI_0_DummyRead(); //normal read mode
  tft.Select_SFI_8_DummyRead(); //1byte dummy cycle
  //Select_SFI_16_DummyRead();
  //Select_SFI_24_DummyRead();

  tft.Select_SFI_Single_Mode();
  //Select_SFI_Dual_Mode0();
  //Select_SFI_Dual_Mode1();

  tft.SPI_Clock_Period(0);

  tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
  tft.SFI_DMA_Transfer_Width_Height(400, 1280);
  tft.SFI_DMA_Source_Width(400);//


  for (i = 0; i < 7; i++)
  {

    tft.Canvas_Image_Start_address(im * 400 * 1280 * 2 * i); //Layer1~6
    tft.Canvas_image_width(400);//
    tft.Active_Window_XY(0, 0);
    tft.Active_Window_WH(400, 1280);

    tft.Foreground_color_65k(Black);
    tft.Line_Start_XY(0, 0);
    tft.Line_End_XY(399, 1247);
    tft.Start_Square_Fill();

    tft.Foreground_color_65k(Blue2);
    tft.Line_Start_XY(0, 1248);
    tft.Line_End_XY(399, 1279);
    tft.Start_Square_Fill();


    tft.Foreground_color_65k(White);
    tft.Background_color_65k(Blue2);
    tft.CGROM_Select_Internal_CGROM();
    tft.Font_Select_12x24_24x24();
    tft.Goto_Text_XY(0, Line52);
    tft.Show_String("Demo Multi Frame Buffer");

    //LCD_DisplayString(0,Line23 ,"  Demo Mulit Frame Buffer",Black,White);




    tft.SFI_DMA_Source_Start_Address(im * 400 * 1280 * 2 * i); //
    tft.Start_SFI_DMA();
    tft.Check_Busy_SFI_DMA();
    //delay(500);
    delay(100);
    tft.Main_Image_Width(400);
    tft.Main_Window_Start_XY(0, 0);
    tft.Main_Image_Start_Address(im * 400 * 1280 * 2 * i); //switch display windows to
  }
  //delay(2000);
  //  Color_Bar_ON();

  //set canvas to 8188
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(3200);
  tft.Main_Window_Start_XY(0, 0);
  tft.Canvas_Image_Start_address(0);
  tft.Canvas_image_width(3200);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(3200, 1280);

  for (im = 0; im < 5; im++)
  {
    tft.Canvas_Image_Start_address(0);//Layer1~6

    tft.Foreground_color_65k(Black);
    tft.Line_Start_XY(0 + im * 400, 0);
    tft.Line_End_XY(399 + im * 400, 1255);
    tft.Start_Square_Fill();

    tft.Foreground_color_65k(Blue2);
    tft.Line_Start_XY(0 + im * 400, 1256);
    tft.Line_End_XY(399 + im * 400, 1279);
    tft.Start_Square_Fill();


    tft.SFI_DMA_Destination_Upper_Left_Corner(0 + im * 400, 0);
    tft.SFI_DMA_Source_Start_Address(im * 1280 * 400 * 2); //
    tft.Start_SFI_DMA();
    tft.Check_Busy_SFI_DMA();
    delay(5);

    /*  Foreground_color_65k(White);
      tft.Background_color_65k(Blue2);
      tft.CGROM_Select_Internal_CGROM();
      tft.Font_Select_12x24_24x24();
      tft.Goto_Text_XY(i*1024,Line52);
      tft.Show_String("Demo Mulit Frame Buffer");
    */
  }
  //  Color_Bar_OFF();

  for (j = 0; j < 4; j++)
  {
    for (i = 0; i < 400; i++)
    {
      tft.Main_Window_Start_XY(i + j * 400, 0);
      delay(5);
    }
    delay(30);
  }

  for (j = 4; j > 0; j--)
  {
    for (i = 0; i < 400; i++)
    {
      tft.Main_Window_Start_XY(j * 400 - i, 0);
      delay(5);
    }

  }

  delay(500);

  NextStep();
}
//-----------------------------------------------------------------------------
void App_Demo_slide_frame_buffer(void)
{ uint32_t im = 1;
  uint16_t i, j;

  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  //DMA initial setting
  tft.Enable_SFlash_SPI();
  tft.Select_SFI_1();
  tft.Select_SFI_DMA_Mode();
  tft.Select_SFI_24bit_Address();

  //Select_SFI_Waveform_Mode_0();
  tft.Select_SFI_Waveform_Mode_3();

  //Select_SFI_0_DummyRead(); //normal read mode
  tft.Select_SFI_8_DummyRead(); //1byte dummy cycle
  //Select_SFI_16_DummyRead();
  //Select_SFI_24_DummyRead();

  tft.Select_SFI_Single_Mode();
  //Select_SFI_Dual_Mode0();
  //Select_SFI_Dual_Mode1();

  tft.SPI_Clock_Period(0);

  tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
  tft.SFI_DMA_Transfer_Width_Height(400, 1280);
  tft.SFI_DMA_Source_Width(400);//


  //Color_Bar_ON();

  //set canvas to 1600
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(800);
  tft.Main_Window_Start_XY(0, 0);
  tft.Canvas_Image_Start_address(0);
  tft.Canvas_image_width(800);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(800, 2560);

  tft.Canvas_Image_Start_address(0);//

  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(799, 2559);
  tft.Start_Square_Fill();

  tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
  tft.SFI_DMA_Source_Start_Address(0);//
  tft.Start_SFI_DMA();
  tft.Check_Busy_SFI_DMA();

  tft.SFI_DMA_Destination_Upper_Left_Corner(400, 0);
  tft.SFI_DMA_Source_Start_Address(im * 400 * 1280 * 2); //
  tft.Start_SFI_DMA();
  tft.Check_Busy_SFI_DMA();

  tft.SFI_DMA_Destination_Upper_Left_Corner(0, 1280);
  tft.SFI_DMA_Source_Start_Address(im * 400 * 1280 * 4); //
  tft.Start_SFI_DMA();
  tft.Check_Busy_SFI_DMA();

  tft.SFI_DMA_Destination_Upper_Left_Corner(400, 1280);
  tft.SFI_DMA_Source_Start_Address(im * 400 * 1280 * 6); //
  tft.Start_SFI_DMA();
  tft.Check_Busy_SFI_DMA();



  //Color_Bar_OFF();


  for (i = 0; i < 800 - 400 + 1; i++)
  {
    tft.Main_Window_Start_XY(i, 0);
    delay(5);
  }
  delay(1);

  for (j = 0; j < 2560 - 1280 + 1; j++)
  {
    tft.Main_Window_Start_XY(400, j);
    delay(5);
  }
  delay(1);


  for (i = 800 - 400; i > 0; i--)
  {
    tft.Main_Window_Start_XY(i, 1280);
    delay(5);
  }
  tft.Main_Window_Start_XY(0, 1280);
  delay(1);

  for (j = 2560 - 1280; j > 0; j--)
  {
    tft.Main_Window_Start_XY(0, j);
    delay(5);
  }
  tft.Main_Window_Start_XY(0, 0);
  delay(5);


  NextStep();
}
//-----------------------------------------------------------------------------
void App_Demo_Alpha_Blending(void)
{ uint32_t im = 1;
  uint16_t i, j;

  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(layer1_start_addr);//
  tft.Main_Image_Width(400);
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);


  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1247);
  tft.Start_Square_Fill();

  tft.Foreground_color_65k(Blue2);
  tft.Line_Start_XY(0, 1248);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  tft.Foreground_color_65k(White);
  tft.Background_color_65k(Blue2);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(0, Line52);
  tft.Show_String(" Demo Alpha Blending");
  tft.Foreground_color_65k(White);
  tft.Background_color_65k(Black);
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(0, Line51);
  tft.Show_String("Fade in and fade out");

  delay(1000);

  //DMA initial setting
  tft.Enable_SFlash_SPI();
  tft.Select_SFI_1();
  tft.Select_SFI_DMA_Mode();
  tft.Select_SFI_24bit_Address();

  //tft.Select_SFI_Waveform_Mode_0();
  tft.Select_SFI_Waveform_Mode_3();

  //tft.Select_SFI_0_DummyRead(); //normal read mode
  tft.Select_SFI_8_DummyRead(); //1byte dummy cycle
  //tft.Select_SFI_16_DummyRead();
  //tft.Select_SFI_24_DummyRead();

  tft.Select_SFI_Single_Mode();
  //tft.Select_SFI_Dual_Mode0();
  //tft.Select_SFI_Dual_Mode1();

  tft.SPI_Clock_Period(0);


  //Clear layer2 to color black
  tft.Canvas_Image_Start_address(layer2_start_addr);//
  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  //DMA picture to layer2
  tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
  tft.SFI_DMA_Transfer_Width_Height(400, 1280);
  tft.SFI_DMA_Source_Width(400);//
  tft.SFI_DMA_Source_Start_Address(0);//
  tft.Start_SFI_DMA();
  tft.Check_Busy_SFI_DMA();


  //Clear layer3 to color black
  tft.Canvas_Image_Start_address(layer3_start_addr);//
  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  //DMA picture to layer3
  tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
  tft.SFI_DMA_Transfer_Width_Height(400, 1280);
  tft.SFI_DMA_Source_Width(400);//
  tft.SFI_DMA_Source_Start_Address(im * 400 * 1280 * 2); //
  tft.Start_SFI_DMA();
  tft.Check_Busy_SFI_DMA();

  //BTE move alpha blending by picture
  tft.BTE_Destination_Color_16bpp();
  tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
  tft.BTE_Destination_Image_Width(400);
  tft.BTE_Destination_Window_Start_XY(0, 0);
  tft.BTE_Window_Size(400, 1280);

  tft.BTE_S0_Color_16bpp();
  tft.BTE_S0_Memory_Start_Address(layer2_start_addr);
  tft.BTE_S0_Image_Width(400);
  tft.BTE_S0_Window_Start_XY(0, 0);

  tft.BTE_S1_Color_16bpp();
  tft.BTE_S1_Memory_Start_Address(layer3_start_addr);
  tft.BTE_S1_Image_Width(400);
  tft.BTE_S1_Window_Start_XY(0, 0);


  tft.BTE_ROP_Code(15);
  tft.BTE_Operation_Code(10); //BTE move

  for (j = 0; j < 4; j += 2)
  {
    //DMA picture to layer2
    tft.Canvas_Image_Start_address(layer2_start_addr);//
    tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
    tft.SFI_DMA_Transfer_Width_Height(400, 1280);
    tft.SFI_DMA_Source_Width(400);//
    tft.SFI_DMA_Source_Start_Address(im * j * 400 * 1280 * 2); //
    tft.Start_SFI_DMA();
    tft.Check_Busy_SFI_DMA();
    //DMA picture to layer3
    tft.Canvas_Image_Start_address(layer3_start_addr);//
    tft.SFI_DMA_Source_Start_Address(im * (j + 1) * 400 * 1280 * 2); //
    tft.Start_SFI_DMA();
    tft.Check_Busy_SFI_DMA();

    for (i = 0; i < 32; i++)
    {
      tft.BTE_Alpha_Blending_Effect(i);
      tft.BTE_Enable();
      delay(200);
    }

    for (i = 32; i > 0; i--)
    {
      tft.BTE_Alpha_Blending_Effect(i);
      tft.BTE_Enable();
      delay(200);
    }
  }

  NextStep();

}
//-----------------------------------------------------------------------------
void mono_Demo(void)
{


  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Canvas_Image_Start_address(0);
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);


  tft.Foreground_color_65k(Red);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
  NextStep();
  tft.Foreground_color_65k(Green);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
  NextStep();
  tft.Foreground_color_65k(Blue);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
  NextStep();
  tft.Foreground_color_65k(Cyan);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
  NextStep();
  tft.Foreground_color_65k(Yellow);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
  NextStep();
  tft.Foreground_color_65k(Magenta);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
  NextStep();
  tft.Foreground_color_65k(White);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
  NextStep();
  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
  NextStep();
}
//-----------------------------------------------------------------------------
void Text_Demo(void)
{
  //   uint16_t i,j;
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(0);//Layer 1
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);


  //   Color_Bar_ON();
  //   NextStep();
  //  Color_Bar_OFF();



  tft.Foreground_color_65k(White);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();


  /////////////////////////////////////////////////////////////////////////////
  //  LCD_DisplayString(0,Line48 ,"EastRising Technology",Blue2,White);
  //  Foreground_color_65k(White);
  //  Background_color_65k(Blue2);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(20, 1000);
  tft.Show_String("www.buydisplay.ocm");
  tft.Foreground_color_65k(Blue2);
  tft.Background_color_65k(White);
  tft.Goto_Text_XY(0, 850);
  tft.Show_String("7.84 inch TFT Module 400*1280 Dots");
  delay(1000);
  //  NextStep();




  tft.Foreground_color_65k(Black);
  tft.Background_color_65k(White);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_8x16_16x16();
  tft.Goto_Text_XY(0, 0);
  tft.Show_String("Embedded 8x16 ASCII Character");

  tft.Font_Select_12x24_24x24();
  tft.Goto_Text_XY(16, 0);
  tft.Show_String("Embedded 12x24 ASCII Character");

  tft.Font_Select_16x32_32x32();
  tft.Goto_Text_XY(40, 0);
  tft.Show_String("Embedded 16x32 ASCII ");

  /*
    tft.Font_Width_X2();
    tft.Goto_Text_XY(0,16);
    tft.Show_String("Character Width Enlarge x2");
    tft.Font_Width_X1();
    tft.Font_Height_X2();
    tft.Goto_Text_XY(0,32);
    tft.Show_String("Character High Enlarge x2");
    tft.Font_Width_X2();
    tft.Font_Height_X2();
    tft.Goto_Text_XY(0,64);
    tft.Show_String("Character Width & High Enlarge x2");
  */

  tft.Font_Select_8x16_16x16();
  tft.Font_Width_X1();
  tft.Font_Height_X1();
  tft.Goto_Text_XY(100, 0);
  tft.Show_String("Supporting Genitop Inc. UNICODE/BIG5/GB etc. Serial Character ROM with 16x16/24x24/32X32 dots Font.");
  tft.Goto_Text_XY(132, 0);
  tft.Show_String("The supporting product numbers are GT21L16TW/GT21H16T1W, GT23L16U2W, GT23L24T3Y/GT23H24T3Y, GT23L24M1Z, and GT23L32S4W/GT23H32S4W, GT23L24F6Y, GT23L24S1W.");



  //Foreground_color_65k(color65k_yellow);
  //Background_color_65k(color65k_purple);

  //Font_Select_UserDefine_Mode();
  //CGROM_Select_Internal_CGROM();
  tft.CGROM_Select_Genitop_FontROM();

  //GTFont_Select_GT21L16TW_GT21H16T1W();
  //GTFont_Select_GT23L16U2W();
  tft.GTFont_Select_GT23L24T3Y_GT23H24T3Y();
  //GTFont_Select_GT23L24M1Z();
  //GTFont_Select_GT23L32S4W_GT23H32S4W();
  //GTFont_Select_GT20L24F6Y();
  //GTFont_Select_GT21L24S1W();
  //GTFont_Select_GT22L16A1Y();

  // No font chip
  /*
    tft.Font_Width_X1();
    tft.Font_Height_X1();
    tft.CGROM_Select_Genitop_FontROM();
    tft.Font_Select_12x24_24x24();
    tft.Select_SFI_0();
    tft.Select_SFI_Font_Mode();
    tft.Select_SFI_24bit_Address();
    tft.Select_SFI_Waveform_Mode_0();
    tft.Select_SFI_0_DummyRead();
    tft.Select_SFI_Single_Mode();
    tft.SPI_Clock_Period(4);   // Freq must setting <=20MHZ

    tft.Enable_SFlash_SPI();

    tft.Foreground_color_65k(color65k_green);
    tft.Background_color_65k(color65k_blue);

    tft.Set_GTFont_Decoder(0x11);  //BIG5

    tft.Goto_Text_XY(200, 0);
    tft.Show_String("Demo GT23L24T3Y BIG5:");
    tft.Font_Select_8x16_16x16();
    tft.Goto_Text_XY(230, 0);
    tft.Show_String("16x16:RA8876 TFT");
    tft.Font_Select_12x24_24x24();
    tft.Goto_Text_XY(254, 0);
    tft.Show_String("24x24:RA8876 TFT");


    tft.Foreground_color_65k(color65k_purple);
    tft.Background_color_65k(color65k_yellow);
    tft.Set_GTFont_Decoder(0x01);  //GB2312
    tft.Goto_Text_XY(280, 0);
    tft.Show_String("Demo GT23L24T3Y GB2312:");
    tft.Font_Select_8x16_16x16();
    tft.Goto_Text_XY(304, 0);
    tft.Show_String("16x16:RA8876 TFT");
    tft.Font_Select_12x24_24x24();
    tft.Goto_Text_XY(328, 0);
    tft.Show_String("16x16:RA8876 TFT");

    tft.Set_GTFont_Decoder(0x11);  //BIG5 */


  tft.Foreground_color_65k(color65k_black);
  tft.Font_Background_select_Transparency();//³]©w­I´º³z©ú¦â
  tft.BTE_Destination_Color_16bpp();

  tft.Active_Window_XY(10, 600);
  tft.Active_Window_WH(80, 80);
  tft.Goto_Pixel_XY(0, 600);

  tft.Show_picture(80 * 80, pic_80x80);
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);

  tft.Goto_Text_XY(0, 580);
  tft.Show_String("text transparent");



  tft.Font_Background_select_Color();  //³]©w­I´º¨Ï¥Î³]©wÃC¦â
  tft.Foreground_color_65k(color65k_black);
  tft.Background_color_65k(color65k_white);
  delay(10000);

  /*    tft.Goto_Text_XY(0,700);
      tft.Show_String("Demo text cursor:");

      tft.Goto_Text_XY(0,724);
      tft.Show_String("0123456789");
      tft.Text_cursor_initial();
     delay(1000);

      for(i=0;i<14;i++)
      {
       delay(100);
       tft.Text_Cursor_H_V(1+i,15-i);
      }
         delay(2000);

      tft.Disable_Text_Cursor();

      tft.CGROM_Select_Internal_CGROM();
      tft.Font_Select_8x16_16x16();

      tft.Foreground_color_65k(color65k_blue);
      tft.Goto_Text_XY(0,484);
      tft.Show_String("Demo graphic cursor:");

      tft.Set_Graphic_Cursor_Color_1(0xff);
      tft.Set_Graphic_Cursor_Color_2(0x00);

      tft.Graphic_cursor_initial();
      tft.Graphic_Cursor_XY(0,508);
      tft.Select_Graphic_Cursor_1();
      delay(2000);
      tft.Select_Graphic_Cursor_2();
      delay(2000);
      tft.Select_Graphic_Cursor_3();
      delay(2000);
      tft.Select_Graphic_Cursor_4();
      delay(2000);
      tft.Select_Graphic_Cursor_2();

      for(j=0;j<2;j++)
      {
       for(i=0;i<400;i++)
       {
        tft.Graphic_Cursor_XY(i,508+j*20);
      delay(2);
       }
      }
       tft.Graphic_Cursor_XY(0,508);

     delay(200);
     tft.Disable_Graphic_Cursor();   */

  NextStep();


}

//-----------------------------------------------------------------------------

void DMA_Demo(void)
{
  uint32_t i;

  ///////////////////////////////////////////////////////////////

  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(0);//Layer 1
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);





  //DMA initial setting
  tft.Enable_SFlash_SPI();
  tft.Select_SFI_1();
  tft.Select_SFI_DMA_Mode();
  tft.Select_SFI_24bit_Address();

  //Select_SFI_Waveform_Mode_0();
  tft.Select_SFI_Waveform_Mode_3();

  //Select_SFI_0_DummyRead(); //normal read mode
  tft.Select_SFI_8_DummyRead(); //1byte dummy cycle
  //Select_SFI_16_DummyRead();
  //Select_SFI_24_DummyRead();

  // Select_SFI_Single_Mode();
  tft.Select_SFI_Dual_Mode0();
  // Select_SFI_Dual_Mode1();

  tft.SPI_Clock_Period(0);



  //DMA initail normally command setting
  //  LCD_CmdWrite(0x01);
  //  LCD_DataWrite(0x83);
  //  LCD_CmdWrite(0xB7);
  //  LCD_DataWrite(0xd4);
  //  LCD_CmdWrite(0xBB);
  //  LCD_DataWrite(0x00);

  /*
    //if used 32bit address Flash ex.256Mbit,512Mbit
    //must be executed following in 24bit address mode to switch to 32bit address mode

    //Select_nSS_drive_on_xnsfcs0();
    Select_nSS_drive_on_xnsfcs1();

    Reset_CPOL();
    //Set_CPOL();
    Reset_CPHA();
    //Set_CPHA();

    //Enter 4-byte mode
    nSS_Active();
    SPI_Master_FIFO_Data_Put(0xB7);  //switch to 32bit address mode
    delayMicroseconds(10);
    nSS_Inactive();

    Select_SFI_32bit_Address();
  */


  tft.SFI_DMA_Destination_Upper_Left_Corner(0, 0);
  tft.SFI_DMA_Transfer_Width_Height(400, 1280);
  tft.SFI_DMA_Source_Width(400);//

  //execute DMA to show 400x1280 picture
  for (i = 0; i < 7; i++)
  {
    tft.SFI_DMA_Source_Start_Address(i * 400 * 1280 * 2); //
    tft.Start_SFI_DMA();
    tft.Check_Busy_SFI_DMA();
    delay(30);
    NextStep();
  }


}
//-----------------------------------------------------------------------------
void Geometric(void)
{
  uint16_t i;
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(0);//Layer 1
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);



  ///////////////////////////Square

  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  for (i = 0; i <= 200; i += 8)
  { tft.Foreground_color_65k(Red);
    tft.Line_Start_XY(0 + i, 0 + i);
    tft.Line_End_XY(399 - i, 1279 - i);
    tft.Start_Square();
    delay(50);
  }

  for (i = 0; i <= 200; i += 8)
  { tft.Foreground_color_65k(Black);
    tft.Line_Start_XY(0 + i, 0 + i);
    tft.Line_End_XY(399 - i, 1279 - i);
    tft.Start_Square();
    delay(50);
  }
  delay(2000);
  ///////////////////////////Square Of Circle
  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  for (i = 0; i <= 160; i += 8)
  { tft.Foreground_color_65k(Green);
    tft.Line_Start_XY(0 + i, 0 + i);
    tft.Line_End_XY(399 - i, 1279 - i);
    tft.Circle_Square_Radius_RxRy(10, 10);
    tft.Start_Circle_Square();
    delay(50);
  }

  for (i = 0; i <= 160; i += 8)
  { tft.Foreground_color_65k(Black);
    tft.Line_Start_XY(0 + i, 0 + i);
    tft.Line_End_XY(399 - i, 1279 - i);
    tft.Circle_Square_Radius_RxRy(10, 10);
    tft.Start_Circle_Square();
    delay(50);
  }
  delay(2000);

  ///////////////////////////Circle
  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  for (i = 0; i <= 200; i += 8)
  { tft.Foreground_color_65k(Blue);
    tft.Circle_Center_XY(400 / 2, 1280 / 2);
    tft.Circle_Radius_R(i);
    tft.Start_Circle_or_Ellipse();
    delay(50);
  }

  for (i = 0; i <= 200; i += 8)
  { tft.Foreground_color_65k(Black);
    tft.Circle_Center_XY(400 / 2, 1280 / 2);
    tft.Circle_Radius_R(i);
    tft.Start_Circle_or_Ellipse();
    delay(50);
  }
  delay(2000);

  ///////////////////////////Ellipse
  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  for (i = 0; i <= 200; i += 8)
  { tft.Foreground_color_65k(White);
    tft.Circle_Center_XY(400 / 2, 1280 / 2);
    tft.Ellipse_Radius_RxRy(i, i + 100);
    tft.Start_Circle_or_Ellipse();
    delay(50);
  }

  for (i = 0; i <= 200; i += 8)
  { tft.Foreground_color_65k(Black);
    tft.Circle_Center_XY(400 / 2, 1280 / 2);
    tft.Ellipse_Radius_RxRy(i, i + 100);
    tft.Start_Circle_or_Ellipse();
    delay(50);
  }
  delay(2000);

  ////////////////////////////Triangle
  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  for (i = 0; i <= 180; i += 8)
  { tft.Foreground_color_65k(Yellow);
    tft.Triangle_Point1_XY(i, 1280 / 2);
    tft.Triangle_Point2_XY(399 - i, i * 2);
    tft.Triangle_Point3_XY(399 - i, 1279 - i * 2);
    tft.Start_Triangle();
    delay(50);
  }

  for (i = 0; i <= 180; i += 8)
  { tft.Foreground_color_65k(Black);
    tft.Triangle_Point1_XY(i, 1280 / 2);
    tft.Triangle_Point2_XY(399 - i, i * 2);
    tft.Triangle_Point3_XY(399 - i, 1279 - i * 2);
    tft.Start_Triangle();
    delay(50);
  }
  delay(2000);


  ////////////////////////////line
  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();

  for (i = 0; i <= 400; i += 8)
  { tft.Foreground_color_65k(Cyan);
    tft.Line_Start_XY(i, 0);
    tft.Line_End_XY(399 - i, 1279);
    tft.Start_Line();
    //delay(50);
  }
  for (i = 0; i <= 1280; i += 8)
  { tft.Foreground_color_65k(Cyan);
    tft.Line_Start_XY(0, 1279 - i);
    tft.Line_End_XY(399, i);
    tft.Start_Line();
    //delay(50);
  }

  /*
    for(i=0;i<=400;i+=8)
    {Foreground_color_65k(Black);
    tft.Line_Start_XY(i,0);
    tft.Line_End_XY(399-i,1279);
    tft.Start_Line();
    delay(50);
    }
    for(i=0;i<=1280;i+=8)
    {Foreground_color_65k(Black);
    tft.Line_Start_XY(0,1279-i);
    tft.Line_End_XY(399,i);
    tft.Start_Line();
    delay(50);
    }
  */
  delay(2000);


}
//-----------------------------------------------------------------------------

void gray(void)
{
  int i, col, line;

  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Canvas_Image_Start_address(0);
  tft.Canvas_image_width(400);//
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);



  col = 0; line = 0;
  for (i = 0; i < 32; i++)
  { tft.Foreground_color_65k(i << 11);
    tft.Line_Start_XY(col, line);
    tft.Line_End_XY(col + 12, line + 213);
    tft.Start_Square_Fill();
    col += 12;
  }
  col = 0; line = 213;
  for (i = 31; i >= 0; i--)
  { tft.Foreground_color_65k(i << 11);
    tft.Line_Start_XY(col, line);
    tft.Line_End_XY(col + 12, line + 213);
    tft.Start_Square_Fill();
    col += 12;
  }

  col = 0; line = 426;
  for (i = 0; i < 64; i++)
  { tft.Foreground_color_65k(i << 5);
    tft.Line_Start_XY(col, line);
    tft.Line_End_XY(col + 6, line + 213);
    tft.Start_Square_Fill();
    col += 6;
  }
  col = 0; line = 639;
  for (i = 63; i >= 0; i--)
  { tft.Foreground_color_65k(i << 5);
    tft.Line_Start_XY(col, line);
    tft.Line_End_XY(col + 6, line + 213);
    tft.Start_Square_Fill();
    col += 6;
  }


  col = 0; line = 852;
  for (i = 0; i < 32; i++)
  { tft.Foreground_color_65k(i);
    tft.Line_Start_XY(col, line);
    tft.Line_End_XY(col + 12, line + 213);
    tft.Start_Square_Fill();
    col += 12;
  }
  col = 0; line = 1065;
  for (i = 31; i >= 0; i--)
  { tft.Foreground_color_65k(i);
    tft.Line_Start_XY(col, line);
    tft.Line_End_XY(col + 12, line + 213);
    tft.Start_Square_Fill();
    col += 12;
  }


  delay(1000);
  NextStep();
}

void Clear_user_font_canvas() {
  // Clear layer 2
  tft.Canvas_Image_Start_address(layer2_start_addr);//
  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
  tft.SFI_DMA_Source_Start_Address(0);//
  tft.Start_SFI_DMA();
  tft.Check_Busy_SFI_DMA();

  tft.BTE_S0_Color_16bpp();

  tft.BTE_S1_Color_16bpp();
}

void Show_user_font(const uint8_t * font, String text, uint16_t x, uint16_t y, uint16_t colour) {
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  uint8_t font_width = font[0];
  uint8_t font_height = font[1];
  uint8_t first_char = font[2];
  uint8_t number_of_char = font[3];

  for (uint8_t c = 0; c < text.length(); c++) {
    if (((uint8_t) text[c] >= first_char) && ((uint8_t) text[c] <= (first_char + number_of_char))) {
      // Show character
      tft.BTE_Destination_Color_16bpp();
      tft.BTE_Destination_Memory_Start_Address(layer2_start_addr);
      tft.BTE_Destination_Image_Width(400);
      tft.BTE_Destination_Window_Start_XY(y, x);
      tft.BTE_Window_Size(font_height, font_width);
      tft.Foreground_color_65k(colour);
      tft.Background_color_65k(color65k_black);
      tft.BTE_ROP_Code(15);
      tft.BTE_Operation_Code(8); //BTE color expansion
      //tft.BTE_Operation_Code(9); //BTE color expansion with chroma key

      tft.BTE_Enable();

      uint16_t font_index = (((uint8_t) text[c] - first_char) * font_height * font_width / 8) + 4;

      uint8_t font_mirror[font_width * font_height / 8] = { 0 };
      for (uint16_t b = 0; b < font_width * font_height; b++) {
        uint8_t line = b % font_height;
        uint8_t pixel = b / font_height;
        uint16_t my_byte = (line * font_width + pixel) / 8;
        uint16_t my_bit = (line * font_width + pixel) % 8;
        if (my_byte < (font_width * font_height / 8)) {
          bool font_bit = bitRead(font[font_index + my_byte], 7 - my_bit);
          if (font_bit == true) bitSet(font_mirror[b / 8], 7 - (b % 8));
        }
      }

      tft.Show_picture(font_width * font_height / 8, font_mirror);
      tft.Check_Mem_WR_FIFO_Empty();
      tft.Check_BTE_Busy();

      x += font_width;
    }
  }

  //BTE memory(move) layer2 to layer1
  //BTE_S0_Color_16bpp();
  tft.BTE_S0_Memory_Start_Address(layer2_start_addr);
  tft.BTE_S0_Image_Width(400);
  tft.BTE_S0_Window_Start_XY(0, 0);

  //BTE_Destination_Color_16bpp();
  tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
  tft.BTE_Destination_Image_Width(400);
  tft.BTE_Destination_Window_Start_XY(0, 0);
  tft.BTE_Window_Size(400, 1280);

  tft.BTE_ROP_Code(12);
  tft.BTE_Operation_Code(2); //BTE move
  tft.BTE_Enable();      //memory copy s0(layer2) to layer1
  tft.Check_BTE_Busy();

  delay(500);
}

void Show2_user_font(const uint8_t * font, String text, uint16_t x, uint16_t y, uint16_t colour) {
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  uint8_t font_width = font[0];
  uint8_t font_height = font[1];
  uint8_t first_char = font[2];
  uint8_t number_of_char = font[3];

  for (uint8_t c = 0; c < text.length(); c++) {
    if (((uint8_t) text[c] >= first_char) && ((uint8_t) text[c] <= (first_char + number_of_char))) {
      // Show character
      tft.BTE_Destination_Color_16bpp();
      tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
      tft.BTE_Destination_Image_Width(400);
      tft.BTE_Destination_Window_Start_XY(y, x);
      tft.BTE_Window_Size(font_height, font_width);
      tft.Foreground_color_65k(colour);
      tft.Background_color_65k(color65k_black);
      tft.BTE_ROP_Code(15);
      tft.BTE_Operation_Code(8); //BTE color expansion
      //tft.BTE_Operation_Code(9); //BTE color expansion with chroma key

      tft.BTE_Enable();

      uint16_t font_index = (((uint8_t) text[c] - first_char) * font_height * font_width / 8) + 4;

      uint8_t font_mirror[font_width * font_height / 8] = { 0 };
      for (uint16_t b = 0; b < font_width * font_height; b++) {
        uint8_t line = b % font_height;
        uint8_t pixel = b / font_height;
        uint16_t my_byte = (line * font_width + pixel) / 8;
        uint16_t my_bit = (line * font_width + pixel) % 8;
        if (my_byte < (font_width * font_height / 8)) {
          bool font_bit = bitRead(font[font_index + my_byte], 7 - my_bit);
          if (font_bit == true) bitSet(font_mirror[b / 8], 7 - (b % 8));
        }
      }

      tft.Show_picture(font_width * font_height / 8, font_mirror);
      tft.Check_Mem_WR_FIFO_Empty();
      tft.Check_BTE_Busy();

      x += font_width;
    }
  }

  //BTE memory(move) layer2 to layer1
  //BTE_S0_Color_16bpp();
  //tft.BTE_S0_Memory_Start_Address(layer2_start_addr);
  //tft.BTE_S0_Image_Width(400);
  //tft.BTE_S0_Window_Start_XY(0, 0);

  //BTE_Destination_Color_16bpp();
  //tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
  //tft.BTE_Destination_Image_Width(400);
  //tft.BTE_Destination_Window_Start_XY(0, 0);
  //tft.BTE_Window_Size(400, 1280);

  //tft.BTE_ROP_Code(12);
  //tft.BTE_Operation_Code(2); //BTE move
  //tft.BTE_Enable();      //memory copy s0(layer2) to layer1
  //tft.Check_BTE_Busy();

  delay(500);
}

void Fix_rinkydink_font(const uint8_t * font) {
  uint8_t font_width = font[0];
  uint8_t font_height = font[1];
  uint8_t first_char = font[2];
  uint8_t number_of_char = font[3];

  // Write header
  Serial.println(" ");
  Serial.println("const uint8_t Font" + String(font_width) + "x" + String(font_height) + "[" + String(font_width * font_height * number_of_char / 8 + 4) + "] PROGMEM = {");
  write_hex_code(font_height);
  write_hex_code(font_width);
  write_hex_code(first_char);
  write_hex_code(number_of_char);
  Serial.println(" ");

  for (uint8_t c = 0; c < number_of_char; c++) {
    uint16_t font_index = (c * font_height * font_width / 8) + 4;

    uint8_t font_mirror[font_width * font_height / 8] = { 0 };
    for (uint16_t b = 0; b < font_width * font_height; b++) {
      uint8_t line = b % font_height;
      uint8_t pixel = b / font_height;
      uint16_t my_byte = (line * font_width + pixel) / 8;
      uint16_t my_bit = (line * font_width + pixel) % 8;
      if (my_byte < (font_width * font_height / 8)) {
        bool font_bit = bitRead(font[font_index + my_byte], 7 - my_bit);
        if (font_bit == true) bitSet(font_mirror[b / 8], 7 - (b % 8));
      }
    }

    for (uint16_t i = 0; i < (font_height * font_width / 8); i++) {
      write_hex_code(font_mirror[i]);
    }
    Serial.print("// ");
    char ch = (char) (c + first_char);
    switch (ch) {
      case ' ':
        Serial.println("space");
        break;
        case '/':
        Serial.println("slash");
        break;
        case '\\':
        Serial.println("backslash");
        break;
      default:
        Serial.println(ch);
        break;
    }
  }

  Serial.println("};");
  Serial.println(" ");
}

void write_hex_code(uint8_t number) {
  Serial.print("0x");
  if (number >= 16) Serial.print(String(number >> 4, HEX));
  else Serial.print("0");
  Serial.print(String(number & 0xF, HEX));
  Serial.print(", ");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
