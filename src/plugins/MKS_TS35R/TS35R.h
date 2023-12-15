/*
 * MKS TS35R connected to MKS DLC32
 *
 * AZ, 2023
 */
#pragma once

typedef enum TS35RCMD{
  TS35RCMD_NONE = 0,
  TS35RCMD_ME,
  TS35RCMD_MW, // move in direction
  TS35RCMD_MN,
  TS35RCMD_MS,
  TS35RCMD_SE, // stop in direction
  TS35RCMD_SW,
  TS35RCMD_SN,
  TS35RCMD_SS,
  TS35RCMD_FIN,  // move focus
  TS35RCMD_FOUT,
  TS35RCMD_FSTOP, // stop focus
  TS35RCMD_STOP, // all stop
  TS35RCMD_TRACK, // toggle tracking
  TS35RCMD_FSPEED, // toggle focus speed
} TS35RCMD;

// I keep it in integer (easy to compare)
typedef struct DispCoordinate {
  uint32_t d; // 0xDDDMMSS
  uint32_t r; // 0xHHMMSS
} DispCoordinate;


class TS35R {
public:
  void init(); // to be called from OnStep
  void loop(); // to be called from OnStep
  TS35RCMD getCmd(); // to be called from loop(), return next command from display (if any)

  void task(); // called from display code only
  void sendCmd(TS35RCMD cmd); // called from display code only
private:
  void startup();
  void drawMain();
  void updateMain();

  void getVersion(char version[]);
  DispCoordinate getMountPosition();
  bool isMoving();
  bool isTracking();
  bool isFastFocusSpeed();

  // my dev setup needs a bit different settings...
  void _init(bool _isdlc32);

  bool           m_isDLC32;
  DispCoordinate m_MountPosition;
  time_t         m_Time;
  QueueHandle_t  m_CmdQueue;
};

extern TS35R ts35r;
