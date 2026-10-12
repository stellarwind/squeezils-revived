// ReadCameraSettings (SQZ.EXE 0x408600): the [section] camera settings of the INI file (generated from the disassembly).
#include "IniReader/IniReader.h"
struct CameraSettings {
    float povOffsetX, povOffsetY, povOffsetZ;                      // 0x00
    int autoLeashMode, lookAtReturnMode;                           // 0x0c
    float leashLength, outLeashSpeed, povLeashSpeed;               // 0x14
    float cameraOutPitch, cameraPovPitch, cameraOutYaw, cameraPovYaw;   // 0x20
    float lookAtAttenPitch, lookAtAttenYaw, lookAtPitchRange, lookAtYawRange;   // 0x30
    float flyPitchRange, flyYawRange;                              // 0x40
    float maxJumpHeight, maxFlyPitchAdjust, maxFlyYawAdjust, maxJumpPitchAdjust;   // 0x48
    float lookAtTurnValue, returnToBaseTime;                       // 0x58
};
extern float gCamInterpTime;                                       // 0x4c6480
void __stdcall ReadCameraSettings(const char *section, CameraSettings *out)
{
    IniReader ini;
    ini.SetSection(section);
    out->povOffsetX = ini.GetFloat("PovOffsetX", 0.0f);
    out->povOffsetY = ini.GetFloat("PovOffsetY", 0.0f);
    out->povOffsetZ = ini.GetFloat("PovOffsetZ", 0.0f);
    out->autoLeashMode = ini.GetInt("AutoLeashMode", 1);
    out->lookAtReturnMode = ini.GetInt("LookAtReturnMode", 1);
    out->leashLength = ini.GetFloat("LeashLength", 1.0f);
    out->outLeashSpeed = ini.GetFloat("OutLeashSpeed", 0.0f);
    out->povLeashSpeed = ini.GetFloat("PovLeashSpeed", 2.0f);
    out->cameraOutPitch = ini.GetFloat("CameraOutPitch", 0.0f);
    out->cameraPovPitch = ini.GetFloat("CameraPovPitch", 0.0f);
    out->cameraOutYaw = ini.GetFloat("CameraOutYaw", 0.0f);
    out->cameraPovYaw = ini.GetFloat("CameraPovYaw", 0.0f);
    out->lookAtAttenPitch = ini.GetFloat("LookAtAttenPitch", 0.0f);
    out->lookAtAttenYaw = ini.GetFloat("LookAtAttenYaw", 0.0f);
    out->lookAtPitchRange = ini.GetFloat("LookAtPitchRange", 0.0f);
    out->lookAtYawRange = ini.GetFloat("LookAtYawRange", 0.0f);
    out->lookAtTurnValue = ini.GetFloat("LookAtTurnValue", 4.0f);
    out->returnToBaseTime = ini.GetFloat("ReturnToBaseTime", 0.4f);
    out->flyPitchRange = ini.GetFloat("FlyPitchRange", 0.0f);
    out->flyYawRange = ini.GetFloat("FlyYawRange", 0.0f);
    out->maxJumpHeight = ini.GetFloat("MaxJumpHeight", 0.0f);
    out->maxFlyPitchAdjust = ini.GetFloat("MaxFlyPitchAdjust", 0.0f);
    out->maxFlyYawAdjust = ini.GetFloat("MaxFlyYawAdjust", 0.0f);
    out->maxJumpPitchAdjust = ini.GetFloat("MaxJumpPitchAdjust", 0.0f);
    ini.SetSection("Camera");
    gCamInterpTime = ini.GetFloat("CamInterpTime", 0.5f);
}
