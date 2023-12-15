/*
 * MKS TS35R connected to MKS DLC32
 *
 * It is running on Core0. OnStepX is not thread safe, commands
 * has to be run on Core1, with SERAIL_LOCAL2 or just executing commands
 * in OnStep task.
 *
 * But I think informational (const) calls are ok directly.
 *
 * Some possible commands:
 *  :GU# - complete status
 *
 * Direct API:
 *  mount.isTracking(), getMountPosition(), isSlewing(), isEnabled()
 *        .tracking(bool), enable(bool)
 *
 * AZ, 2023
 */
#include "../../Common.h"
#include "../../lib/tasks/OnTask.h"
#include "../../telescope/Telescope.h"
#include "../../telescope/mount/Mount.h"
#include "../../telescope/mount/guide/Guide.h"
#include "../../telescope/focuser/Focuser.h"
#include "../../lib/axis/Axis.h"
#include "TS35R.h"

void TS35RWrapper() { ts35r.loop(); }

void TS35R::init() {
  VLF("MSG: Plugins, starting: TS35R");

#ifdef MKS_DLC32_DEV
  _init(false);
#else
  _init(true);
#endif
  // half a second is acceptable latancy for what I plan at the moment...
  tasks.add(500, 0, true, 7, TS35RWrapper);
}


static Coordinate _mount_position; // to safe access from Core0

void TS35R::getVersion(char version[]) {
  *version = 0;
  bool sf, nr;
  CommandError err;
  telescope.command(version, "GV", "N", &sf, &nr, &err);
  if(!*version)
    strcpy(version, "Unknown");
}

// from Convert::doubleToHms/Dms
static uint32_t _encode_position(double v){
  bool neg = false;
  if(v < 0){
    neg = true;
    v = -v;
  }
  v += 0.000139; // round to 0.5sec
  double fHD = floor(v); // hours or degree
  double fMinute = (v - fHD)*60.0;
  double fSecond = (fMinute - floor(fMinute))*60.0;
  int iHD = (int)fHD; // can be negative
  int iMinute = (int)fMinute;
  int iSecond = (int)fSecond;
  return ((uint32_t)iHD<<16)|((uint32_t)iMinute<<8)|((uint32_t)iSecond)|((neg?0x80000000:0));
}

DispCoordinate TS35R::getMountPosition() {
  DispCoordinate dc;
  // getPosition() call update, other can also... we better use a copy.
  dc.r = _encode_position(radToHrs(_mount_position.r));
  dc.d = _encode_position(radToDeg(_mount_position.d));
  return dc;
}

bool TS35R::isMoving(){
  return mount.isSlewing();
}

bool TS35R::isTracking(){
  return mount.isTracking();
}

/*
 * All OnStep Focuser methods are private,
 * so communication is possible with commands only.
 * :F[n]# - (n = 1..4) set move rate 1um, 10um, 100um, 1/2goto rate. Use setMoveRate()
 * :FW# - get GoTo rate (in um)
 * :FT# - get status and GoTo speed (as an index)
 * :FQ# - stop
 * :F+# and :F-# - move
 *
 * There is NO getMoveRate.
 * So my strategy set the rate every time we move (it is just one assign)
 *
 * bool Focuser::command(char *reply, char *command, char *parameter, 
 *                       bool *supressFrame, bool *numericReply, CommandError *commandError) {
 */

uint32_t _focuser_move_rate = 1; // valid values 1 - 4, not really working, always 0.1mm...

bool TS35R::isFastFocusSpeed(){
  return _focuser_move_rate >= 3;
}

static void _focuser_toggle_speed(){
  if(_focuser_move_rate >= 3)
    _focuser_move_rate = 1;
  else
    _focuser_move_rate = 3;
  // set right before moving
}

static void _focuser_move(bool move_in){
  char reply[32], cmd[3];
  bool sf, nr;
  CommandError err;
  cmd[0] = 'F'; // both used commands starts with 'F'
  cmd[2] = 0; // both used commands have length 2
  // set rate
  cmd[1] = _focuser_move_rate + '0';
  focuser.command(reply, cmd, "", &sf, &nr, &err);
  // move
  cmd[1] = (move_in ? '+' : '-');
  focuser.command(reply, cmd, "", &sf, &nr, &err);
}

static void _focuser_stop(){
  char reply[32];
  bool sf, nr;
  CommandError err;
  focuser.command(reply, "FQ", "", &sf, &nr, &err);
}

static void _guide_cmd(const char *cmd){
  char reply[32], _cmd[16];
  bool sf, nr;
  CommandError err;
  strcpy(_cmd, cmd);
  guide.command(reply, _cmd, "", &sf, &nr, &err);
}

static void _mount_cmd(const char *cmd){
  char reply[32], _cmd[16];
  bool sf, nr;
  CommandError err;
  strcpy(_cmd, cmd);
  mount.command(reply, _cmd, "", &sf, &nr, &err);
}

void TS35R::loop() {
  // may be getMountPosition(CR_MOUNT or something else)
  // getPosition calls update, unsafe to call directly from Core0 
  // _mount_position = mount.getPosition(); // that is mount position "in cosmos"
  // I want mount position as should be seend on the mount... See :GX4[n]# commands
  _mount_position.r = axis1.getInstrumentCoordinate();
  _mount_position.d = axis2.getInstrumentCoordinate();

  TS35RCMD cmd;
  while((cmd = getCmd())){
    // Serial.printf("Command %u\n", cmd);
    switch(cmd){
      case TS35RCMD_ME:
        _guide_cmd("Me");
        break;
      case TS35RCMD_MW:
        _guide_cmd("Mw");
        break;
      case TS35RCMD_MN:
        _guide_cmd("Mn");
        break;
      case TS35RCMD_MS:
        _guide_cmd("Ms");
        break;
      case TS35RCMD_SE:
        _guide_cmd("Qe");
        break;
      case TS35RCMD_SW:
        _guide_cmd("Qw");
        break;
      case TS35RCMD_SN:
        _guide_cmd("Qn");
        break;
      case TS35RCMD_SS:
        _guide_cmd("Qs");
        break;
      case TS35RCMD_FIN:
        _focuser_move(true);
        break;
      case TS35RCMD_FOUT:
        _focuser_move(false);
        break;
      case TS35RCMD_FSTOP:
        _focuser_stop();
        break;
      case TS35RCMD_STOP:
        _guide_cmd("Q");
        break;
      case TS35RCMD_TRACK:
        _mount_cmd(mount.isTracking()? "Td" : "Te");
        break;
      case TS35RCMD_FSPEED:
        _focuser_toggle_speed();
        break;
      default:
        break;
    }
  }
}
