/*
 * MKS TS35R connected to MKS DLC32
 *
 * It is running on Core0. OnStepX is not thread safe, commands
 * has to be run on Core1, with SERAIL_LOCAL2 or just executing commands
 * in OnStep task.
 *
 * But I think informational (const) calls are ok directly.
 *
 * Interesting commands:
 *  :Q# - stop/abort
 *  :Te# / :Td# - enable/disable tracking
 *  :GU# - complete status
 *
 * Direct API:
 *  mount.isTracking(), getMountPosition(), isSlewing(), isEnabled()
 *        .tracking(bool), enable(bool)
 *
 * AZ, 2023
 */
#include <LovyanGFX.hpp>
//#include "LGFX_ESP32_MKS_DLC32.hpp"
// With OnStepX display is periodically blinking. May be just insufficient power, but may be PWM.
// So I have decided to disable light control and do this manually. And it seems like that helps,
// I no longer see flackering.
#include "LGFX_ESP32_MKS_DLC32_no_light.hpp"
// Should be correct pin
static const uint8_t TS35R_LIGHT_PIN = 5;
#include "FreeSans18pt7b_deg.h"

#include "TS35R.h"
#include "TS35RButton.h"

LGFX gfx;
TS35R ts35r;
TS35RBtn btnN, btnS, btnW, btnE, btnIn, btnOut, btnFSpeed, btnStop, btnTrack;

TaskHandle_t _ts35rTaskHandle;

static void _drawString(const char *s, textdatum_t datum, uint32_t x, uint32_t y, uint32_t scale, int fg, int bg){
  auto style = gfx.getTextStyle();
  auto font = gfx.getFont();
  gfx.setFont(&FreeSans18pt7b_deg);
  gfx.setTextSize(scale);
  gfx.setTextColor(fg, bg);
  gfx.setTextDatum(datum);
  gfx.drawString(s, x, y);
  gfx.display();
  gfx.setFont(font);
  gfx.setTextStyle(style);
}

void TS35R::startup(){
  gfx.fillScreen(TFT_BLACK);
  _drawString("OnStepX", TL_DATUM, 5, 50, 3, TFT_WHITE, TFT_BLACK);
  String version = "Version ";
  char v[80];
  ts35r.getVersion(v);
  version += v;
  _drawString(version.c_str(), TL_DATUM, 5, 200, 1, TFT_RED, TFT_BLACK);
  delay(5000);
}

void TS35R::drawMain(){
  gfx.fillScreen(TFT_BLACK);
  _drawString("R:", R_BASELINE, 35, 25, 1, TFT_ORANGE, TFT_BLACK);
  _drawString("D:", R_BASELINE, 210, 25, 1, TFT_ORANGE, TFT_BLACK);
  btnN.draw();
  btnS.draw();
  btnW.draw();
  btnE.draw();
  btnIn.draw();
  btnOut.draw();
  btnFSpeed.draw();
  btnStop.draw();
  btnTrack.draw();
}

void TS35R::updateMain(){
  DispCoordinate c = getMountPosition();
  if((c.r>>8) != (m_MountPosition.r>>8)){
    m_MountPosition.r = c.r;
    char s[64];
    sprintf(s, "%c%02uh%02u'", (c.r&0x80000000 ? '-' : ' '), (c.r>>16)&0x7fff, (c.r>>8) & 0xff);
    _drawString(s, L_BASELINE, 40, 25, 1, TFT_ORANGE, TFT_BLACK);
  }
  if((c.d>>8) != (m_MountPosition.d>>8)){
    m_MountPosition.d = c.d;
    char s[64];
    sprintf(s, "%c%u`%02u' ", (c.d&0x80000000 ? '-' : ' '), (c.d>>16)&0x7fff, (c.d>>8) & 0xff);
    _drawString(s, L_BASELINE, 215, 25, 1, TFT_ORANGE, TFT_BLACK);
  }
  time_t now;
  time(&now);
  if(now != m_Time){
    m_Time = now;
    struct tm ti;
    localtime_r(&now, &ti);
    char s[64];
    sprintf(s, "%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
    _drawString(s, R_BASELINE, 478, 25, 1, TFT_ORANGE, TFT_BLACK);
  }
  btnStop.set(isMoving());
  btnTrack.set(isTracking());
  btnFSpeed.set(isFastFocusSpeed());
}

void TS35R::task(){
  gfx.init();
  gfx.setRotation(m_isDLC32 ? 1 : 3);
  gfx.setBrightness(128); // 128, checked and it works
  gfx.setColorDepth(16); // RGB565, 2bytes per pixel

  // Default font
  gfx.setFont(&FreeSans18pt7b_deg); // &fonts::Font4

  startup();
  drawMain();
  uint16_t x, y;
  for(;;) {
    if(gfx.getTouch(&x, &y))
      ts35rtouch.touched(x, y);
    else
      ts35rtouch.released();
    updateMain();  
    delay(50);
  }
}

void TS35R::sendCmd(TS35RCMD cmd){
  if(cmd != TS35RCMD_NONE){
    uint32_t uCmd = static_cast<uint32_t>(cmd);
    xQueueGenericSend( m_CmdQueue, &uCmd, ( TickType_t ) 10, queueSEND_TO_BACK);
    // I ignore errors... 
  }
}

TS35RCMD TS35R::getCmd(){
  uint32_t uCmd;
  if( xQueueReceive(m_CmdQueue, &uCmd, ( TickType_t )0) )
    return static_cast<TS35RCMD>(uCmd);
  return TS35RCMD_NONE;
}

void _ts35rTask(void *_disp) {
  ts35r.task();
}

void TS35R::_init(bool _isdlc32){
  m_isDLC32 = _isdlc32;
  m_MountPosition.r = 0xffffffff; // undefined
  m_MountPosition.d = 0xffffffff; // undefined
  m_Time = 0; // undefined

  btnN.init(&gfx, TS35RBTN_MOMENTARY, TS35RCMD_MN , TS35RCMD_SN ,
            110,  40, 100, 100, TFT_YELLOW, TFT_BLACK, TFT_ORANGE, &FreeSans18pt7b_deg, "N"); 
  btnS.init(&gfx, TS35RBTN_MOMENTARY, TS35RCMD_MS , TS35RCMD_SS ,
            110, 145, 100, 100, TFT_YELLOW, TFT_BLACK, TFT_ORANGE, &FreeSans18pt7b_deg, "S"); 
  btnW.init(&gfx, TS35RBTN_MOMENTARY, TS35RCMD_MW , TS35RCMD_SW ,
              5,  93, 100, 100, TFT_YELLOW, TFT_BLACK, TFT_ORANGE, &FreeSans18pt7b_deg, "W"); 
  btnE.init(&gfx, TS35RBTN_MOMENTARY, TS35RCMD_ME , TS35RCMD_SE ,
            215,  93, 100, 100, TFT_YELLOW, TFT_BLACK, TFT_ORANGE, &FreeSans18pt7b_deg, "E"); 
  btnIn.init(&gfx, TS35RBTN_MOMENTARY, TS35RCMD_FIN , TS35RCMD_FSTOP ,
              5, 250, 152,  65, TFT_YELLOW, TFT_BLACK, TFT_ORANGE, &FreeSans18pt7b_deg, "In"); 
  btnOut.init(&gfx,TS35RBTN_MOMENTARY, TS35RCMD_FOUT , TS35RCMD_FSTOP ,
            162, 250, 153,  65, TFT_YELLOW, TFT_BLACK, TFT_ORANGE, &FreeSans18pt7b_deg, "Out"); 
  btnStop.init(&gfx,TS35RBTN_TOGGLE, TS35RCMD_STOP , TS35RCMD_NONE ,
            340,  40, 135,  65, TFT_YELLOW, TFT_BLACK, TFT_RED, &FreeSans18pt7b_deg, "Stop"); 
  btnTrack.init(&gfx,TS35RBTN_TOGGLE, TS35RCMD_TRACK , TS35RCMD_NONE ,
            340, 145, 135,  65, TFT_YELLOW, TFT_BLACK, TFT_GREEN, &FreeSans18pt7b_deg, "Track"); 
  btnFSpeed.init(&gfx, TS35RBTN_TOGGLE, TS35RCMD_FSPEED , TS35RCMD_NONE ,
            340, 250, 135,  65, TFT_YELLOW, TFT_BLACK, TFT_BLUE, &FreeSans18pt7b_deg, "Speed"); 

  auto light = reinterpret_cast<lgfx::Light_PWM *>(gfx.light());
  if(light){
    auto cfg = light->config();
    cfg.invert = m_isDLC32;
    light->config(cfg);
  } else { // do this manually
    pinMode(TS35R_LIGHT_PIN, OUTPUT);
    digitalWrite(TS35R_LIGHT_PIN, !m_isDLC32);
  }
  auto touch = reinterpret_cast<lgfx::Touch_XPT2046 *>(gfx.touch());
  if(touch){
    auto cfg = touch->config();
    cfg.offset_rotation = m_isDLC32 ? 6 : 6; // I probably have to change that
    touch->config(cfg);
  }
  m_CmdQueue = xQueueCreate( 10, sizeof( uint32_t ) ); // 10 should be more then sufficient, if OnStep is still alive...

  xTaskCreatePinnedToCore(_ts35rTask, "TS35RTask", 10000, this, tskIDLE_PRIORITY + 1, &_ts35rTaskHandle, 0);
}
