/*
 * Small button helpers
 *
 * AZ 2023
 */

class TS35RBtn;

typedef enum TS35RBTN_TYPE{
  TS35RBTN_TOGGLE = 0,
  TS35RBTN_MOMENTARY
} TS35RBTN_TYPE;

// for touch processing
static TS35RBtn *_all_buttons = nullptr;

class TS35RBtn : public LGFX_Button {
  public:
    template<typename T>
    void init(LovyanGFX *_gfx, TS35RBTN_TYPE type, TS35RCMD ontouch, TS35RCMD onrelease, int16_t x, int16_t y, uint16_t w, uint16_t h, 
              const T& outline, const T& fill, const T& textcolor, const GFXfont *font, const char *label){
      initButtonUL(_gfx, x, y, w, h, outline, fill, textcolor, label);
      m_gfx = _gfx;
      m_Type = type;
      m_Font = font;
      m_Next = _all_buttons;
      m_OnTouch = ontouch;
      m_OnRelease = onrelease;

      m_On = false;
      _all_buttons = this;
    }
    void draw(){
      auto font = m_gfx->getFont();
      m_gfx->setFont(m_Font);
      drawButton(m_Type == TS35RBTN_TOGGLE ? m_On : isPressed());
      m_gfx->setFont(font);
    }
    void set(bool on){
      if(on == m_On)
        return;
      m_On = on;
      draw();
    }
    TS35RBtn *next() { return m_Next; }
    TS35RBTN_TYPE type() const { return m_Type; }
    TS35RCMD getOnTouch() const { return m_OnTouch; }
    TS35RCMD getOnRelease() const { return m_OnRelease; }
  private:
    LovyanGFX *m_gfx; // original class does not expose it
    TS35RBTN_TYPE m_Type;
    const GFXfont *m_Font;
    TS35RCMD m_OnTouch;
    TS35RCMD m_OnRelease;

    bool m_On;
    class TS35RBtn *m_Next;
};

const unsigned long TS35RTOUCH_DEBOUNCE = 250;

class TS35RTouch {
public:
  TS35RTouch() : m_TouchedBtn(nullptr), m_TouchTime(0) {}

  void touched(uint16_t x, uint16_t y){
    if(x >= 480)
      x = 479;
    if(y >= 320)
      y = 319;
    unsigned long now = millis();
    if(m_TouchedBtn){
      if(m_TouchedBtn->contains(x, y)){
        m_TouchTime = now;
        return;
      } else if((now - m_TouchTime) < TS35RTOUCH_DEBOUNCE){
        return; // ignore for a while
      } else {
        _btn_released();
        // continue checking other buttons
      }
    }
    TS35RBtn *btn;
    for(btn = _all_buttons; btn; btn = btn->next())
      if(btn->contains(x, y))
        break;
    if(btn){
      btn->press(true);
      m_TouchedBtn = btn;
      m_TouchTime = now;
      if(m_TouchedBtn->type() != TS35RBTN_TOGGLE)
        m_TouchedBtn->draw();
      ts35r.sendCmd(m_TouchedBtn->getOnTouch());
    }
  }

  void released(){
    if(!m_TouchedBtn)
      return; // not interesting
    unsigned long now = millis();
    if((now - m_TouchTime) < TS35RTOUCH_DEBOUNCE)
      return; // ignore for a while
    _btn_released();
  }

private:
  TS35RBtn *m_TouchedBtn;
  unsigned long m_TouchTime;

  void _btn_released(){
    m_TouchedBtn->press(false);
    if(m_TouchedBtn->type() != TS35RBTN_TOGGLE)
      m_TouchedBtn->draw();
    ts35r.sendCmd(m_TouchedBtn->getOnRelease());
    m_TouchedBtn = nullptr;
  }
};

TS35RTouch ts35rtouch;