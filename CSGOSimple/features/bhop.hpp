#pragma once

class C_BasePlayer;
class CUserCmd;
class QAngle;

namespace BunnyHop
{
    void OnCreateMove(CUserCmd* cmd);
    void AutoStrafe(CUserCmd* cmd, QAngle va);
}
extern QAngle unpredangle;