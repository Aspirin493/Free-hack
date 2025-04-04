#include "Math.hpp"
#include <algorithm>
#include "../valve_sdk/csgostructs.hpp"
#include <d3dx9.h>
#include <D3dx9math.h>

namespace Math
{
	//--------------------------------------------------------------------------------
	float VectorDistance(const Vector& v1, const Vector& v2)
	{
		return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
	}
	//--------------------------------------------------------------------------------
	QAngle CalcAngle(const Vector& src, const Vector& dst)
	{
		QAngle vAngle;
		Vector delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
		double hyp = sqrt(delta.x*delta.x + delta.y*delta.y);

		vAngle.pitch = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
		vAngle.yaw = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
		vAngle.roll = 0.0f;

		if (delta.x >= 0.0)
			vAngle.yaw += 180.0f;

		return vAngle;
	}
	//--------------------------------------------------------------------------------
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle)
	{
		Vector ang, aim;

		AngleVectors(viewAngle, aim);
		AngleVectors(aimAngle, ang);

		auto res = RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
		if (std::isnan(res))
			res = 0.f;
		return res;
	}
    //--------------------------------------------------------------------------------
    void ClampAngles(QAngle& angles)
    {
        if(angles.pitch > 89.0f) angles.pitch = 89.0f;
        else if(angles.pitch < -89.0f) angles.pitch = -89.0f;

        if(angles.yaw > 180.0f) angles.yaw = 180.0f;
        else if(angles.yaw < -180.0f) angles.yaw = -180.0f;

        angles.roll = 0;
    }
    //--------------------------------------------------------------------------------
    void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out)
    {
        out[0] = in1.Dot(in2[0]) + in2[0][3];
        out[1] = in1.Dot(in2[1]) + in2[1][3];
        out[2] = in1.Dot(in2[2]) + in2[2][3];
    }
    //--------------------------------------------------------------------------------
    void AngleVectors(const QAngle &angles, Vector& forward)
    {
        float	sp, sy, cp, cy;

        DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
        DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));

        forward.x = cp*cy;
        forward.y = cp*sy;
        forward.z = -sp;
    }
    //--------------------------------------------------------------------------------
    void AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up)
    {
        float sr, sp, sy, cr, cp, cy;

        DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
        DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));
        DirectX::XMScalarSinCos(&sr, &cr, DEG2RAD(angles[2]));

        forward.x = (cp * cy);
        forward.y = (cp * sy);
        forward.z = (-sp);
        right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
        right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
        right.z = (-1 * sr * cp);
        up.x = (cr * sp * cy + -sr*-sy);
        up.y = (cr * sp * sy + -sr*cy);
        up.z = (cr * cp);
    }
    //--------------------------------------------------------------------------------
    void VectorAngles(const Vector& forward, QAngle& angles)
    {
        float tmp, yaw, pitch;

        if (forward[1] == 0 && forward[0] == 0) {
            yaw = 0;
            if (forward[2] > 0)
                pitch = 270;
            else
                pitch = 90;
        } else {
            yaw = (atan2(forward[1], forward[0]) * 180 / DirectX::XM_PI);
            if(yaw < 0)
                yaw += 360;

            tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
            pitch = (atan2(-forward[2], tmp) * 180 / DirectX::XM_PI);
            if (pitch < 0)
                pitch += 360;
        }

        angles[0] = pitch;
        angles[1] = yaw;
        angles[2] = 0;
    }
    //--------------------------------------------------------------------------------
    void VectorAngles(const Vector& forward, Vector& angles)
    {
        Vector view;

        if (!forward[0] && !forward[1])
        {
            view[0] = 0.0f;
            view[1] = 0.0f;
        }
        else
        {
            view[1] = atan2(forward[1], forward[0]) * 180.0f / M_PI;

            if (view[1] < 0.0f)
                view[1] += 360.0f;

            view[2] = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
            view[0] = atan2(forward[2], view[2]) * 180.0f / M_PI;
        }

        angles[0] = -view[0];
        angles[1] = view[1];
        angles[2] = 0.f;
    }
    //--------------------------------------------------------------------------------
    static bool screen_transform(const Vector& in, Vector& out)
    {
        static auto& w2sMatrix = g_EngineClient->WorldToScreenMatrix();

        out.x = w2sMatrix.m[0][0] * in.x + w2sMatrix.m[0][1] * in.y + w2sMatrix.m[0][2] * in.z + w2sMatrix.m[0][3];
        out.y = w2sMatrix.m[1][0] * in.x + w2sMatrix.m[1][1] * in.y + w2sMatrix.m[1][2] * in.z + w2sMatrix.m[1][3];
        out.z = 0.0f;

        float w = w2sMatrix.m[3][0] * in.x + w2sMatrix.m[3][1] * in.y + w2sMatrix.m[3][2] * in.z + w2sMatrix.m[3][3];

        if(w < 0.001f) {
            out.x *= 100000;
            out.y *= 100000;
            return false;
        }

        out.x /= w;
        out.y /= w;

        return true;
    }
    //--------------------------------------------------------------------------------
    bool WorldToScreen(const Vector& in, Vector& out)
    {
        if(screen_transform(in, out)) {
            int w, h;
            g_EngineClient->GetScreenSize(w, h);

            out.x = (w / 2.0f) + (out.x * w) / 2.0f;
            out.y = (h / 2.0f) - (out.y * h) / 2.0f;

            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------------
    void FixAngles(QAngle& angles)
    {
        Normalize3(angles);
        ClampAngles(angles);
    }
    //--------------------------------------------------------------------------------
    void MovementFix(CUserCmd* m_Cmd, QAngle wish_angle, QAngle old_angles) {
        if (old_angles.pitch != wish_angle.pitch || old_angles.yaw != wish_angle.yaw || old_angles.roll != wish_angle.roll) {
            Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

            auto viewangles = old_angles;
            auto movedata = Vector(m_Cmd->forwardmove, m_Cmd->sidemove, m_Cmd->upmove);
            viewangles.Normalize();

            if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && viewangles.roll != 0.f)
                movedata.y = 0.f;

            AngleVectors(wish_angle, wish_forward, wish_right, wish_up);
            AngleVectors(viewangles, cmd_forward, cmd_right, cmd_up);

            auto v8 = sqrt(wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y), v10 = sqrt(wish_right.x * wish_right.x + wish_right.y * wish_right.y), v12 = sqrt(wish_up.z * wish_up.z);

            Vector wish_forward_norm(1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f),
                wish_right_norm(1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f),
                wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up.z);

            auto v14 = sqrt(cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y), v16 = sqrt(cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y), v18 = sqrt(cmd_up.z * cmd_up.z);

            Vector cmd_forward_norm(1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f),
                cmd_right_norm(1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f),
                cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up.z);

            auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

            Vector correct_movement;
            correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28) + (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27);
            correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28) + (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27);
            correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28) + (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27);

            correct_movement.x = std::clamp(correct_movement.x, -450.f, 450.f);
            correct_movement.y = std::clamp(correct_movement.y, -450.f, 450.f);
            correct_movement.z = std::clamp(correct_movement.z, -320.f, 320.f);

            m_Cmd->forwardmove = correct_movement.x;
            m_Cmd->sidemove = correct_movement.y;
            m_Cmd->upmove = correct_movement.z;

            m_Cmd->buttons &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);
            if (m_Cmd->sidemove != 0.0) {
                if (m_Cmd->sidemove <= 0.0)
                    m_Cmd->buttons |= IN_MOVELEFT;
                else
                    m_Cmd->buttons |= IN_MOVERIGHT;
            }

            if (m_Cmd->forwardmove != 0.0) {
                if (m_Cmd->forwardmove <= 0.0)
                    m_Cmd->buttons |= IN_BACK;
                else
                    m_Cmd->buttons |= IN_FORWARD;
            }
        }
    }
    //--------------------------------------------------------------------------------
    void MovementFix(CUserCmd* cmd, QAngle& wishangle)
    {
        Vector view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
        Math::AngleVectors(wishangle, view_fwd, view_right, view_up);
        Math::AngleVectors(cmd->viewangles, cmd_fwd, cmd_right, cmd_up);

        const auto v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
        const auto v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
        const auto v12 = sqrtf(view_up.z * view_up.z);

        const Vector norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
        const Vector norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
        const Vector norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);

        const auto v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
        const auto v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
        const auto v18 = sqrtf(cmd_up.z * cmd_up.z);

        const Vector norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
        const Vector norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
        const Vector norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);

        const auto v22 = norm_view_fwd.x * cmd->forwardmove;
        const auto v26 = norm_view_fwd.y * cmd->forwardmove;
        const auto v28 = norm_view_fwd.z * cmd->forwardmove;
        const auto v24 = norm_view_right.x * cmd->sidemove;
        const auto v23 = norm_view_right.y * cmd->sidemove;
        const auto v25 = norm_view_right.z * cmd->sidemove;
        const auto v30 = norm_view_up.x * cmd->upmove;
        const auto v27 = norm_view_up.z * cmd->upmove;
        const auto v29 = norm_view_up.y * cmd->upmove;

        cmd->forwardmove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25))
            + (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)))
            + (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
        cmd->sidemove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25))
            + (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)))
            + (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));
        cmd->upmove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25))
            + (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28)))
            + (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));

        const auto ratio = 2.f - fmaxf(fabsf(cmd->sidemove), fabsf(cmd->forwardmove)) / 450.f;
        cmd->forwardmove *= ratio;
        cmd->sidemove *= ratio;

        wishangle = cmd->viewangles;
    }
    //--------------------------------------------------------------------------------
    float NormalizeYaw(float yaw)
    {
        while (yaw < -180.f)
            yaw += 360.f;
        while (yaw > 180.f)
            yaw -= 360.f;
        return yaw;
    }
    //--------------------------------------------------------------------------------
    bool IntersectionBoundingBox(const Vector& src, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point) {
        /*
        Fast Ray-Box Intersection
        by Andrew Woo
        from "Graphics Gems", Academic Press, 1990
        */

        constexpr auto NUMDIM = 3;
        constexpr auto RIGHT = 0;
        constexpr auto LEFT = 1;
        constexpr auto MIDDLE = 2;

        bool inside = true;
        char quadrant[NUMDIM];
        int i;

        // Rind candidate planes; this loop can be avoided if
        // rays cast all from the eye(assume perpsective view)
        Vector candidatePlane;
        for (i = 0; i < NUMDIM; i++) {
            if (src[i] < min[i]) {
                quadrant[i] = LEFT;
                candidatePlane[i] = min[i];
                inside = false;
            }
            else if (src[i] > max[i]) {
                quadrant[i] = RIGHT;
                candidatePlane[i] = max[i];
                inside = false;
            }
            else {
                quadrant[i] = MIDDLE;
            }
        }

        // Ray origin inside bounding box
        if (inside) {
            if (hit_point)
                *hit_point = src;
            return true;
        }

        // Calculate T distances to candidate planes
        Vector maxT;
        for (i = 0; i < NUMDIM; i++) {
            if (quadrant[i] != MIDDLE && dir[i] != 0.f)
                maxT[i] = (candidatePlane[i] - src[i]) / dir[i];
            else
                maxT[i] = -1.f;
        }

        // Get largest of the maxT's for final choice of intersection
        int whichPlane = 0;
        for (i = 1; i < NUMDIM; i++) {
            if (maxT[whichPlane] < maxT[i])
                whichPlane = i;
        }

        // Check final candidate actually inside box
        if (maxT[whichPlane] < 0.f)
            return false;

        for (i = 0; i < NUMDIM; i++) {
            if (whichPlane != i) {
                float temp = src[i] + maxT[whichPlane] * dir[i];
                if (temp < min[i] || temp > max[i]) {
                    return false;
                }
                else if (hit_point) {
                    (*hit_point)[i] = temp;
                }
            }
            else if (hit_point) {
                (*hit_point)[i] = candidatePlane[i];
            }
        }

        // ray hits box
        return true;
    }
}
