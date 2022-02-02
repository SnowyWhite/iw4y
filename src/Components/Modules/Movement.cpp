#include "STDInclude.hpp"

namespace Components
{
    Dvar::Var Movement::PlayerDuckedSpeedScale;
    Dvar::Var Movement::PlayerLastStandCrawlSpeedScale;
    Dvar::Var Movement::PlayerProneSpeedScale;
    Dvar::Var Movement::PlayerSpectateSpeedScale;
    Dvar::Var Movement::CGUfoScaler;
    Dvar::Var Movement::CGNoclipScaler;
    Dvar::Var Movement::BGBouncesAllAngles;
    Game::dvar_t* Movement::BGBounces;

    float Movement::PM_CmdScaleForStance(const Game::pmove_s* pm)
    {
        assert(pm->ps != nullptr);

        const auto* playerState = pm->ps;
        float scale;

        if (playerState->viewHeightLerpTime != 0 && playerState->viewHeightLerpTarget == 0xB)
        {
            scale = pm->cmd.serverTime - playerState->viewHeightLerpTime / 400.0f;

            if (0.0f <= scale)
            {
                if (scale > 1.0f)
                {
                    scale = 1.0f;
                    return scale * 0.15f + (1.0f - scale) * 0.65f;
                }

                if (scale != 0.0f)
                {
                    return scale * 0.15f + (1.0f - scale) * 0.65f;
                }
            }
        }

        if ((playerState->viewHeightLerpTime != 0 && playerState->viewHeightLerpTarget == 0x28) &&
            playerState->viewHeightLerpDown == 0)
        {
            scale = 400.0f / pm->cmd.serverTime - playerState->viewHeightLerpTime;

            if (0.0f <= scale)
            {
                if (scale > 1.0f)
                {
                    scale = 1.0f;
                }
                else if (scale != 0.0f)
                {
                    return scale * 0.65f + (1.0f - scale) * 0.15f;
                }
            }
        }

        scale = 1.0f;
        const auto stance = Game::PM_GetEffectiveStance(playerState);

        if (stance == Game::PM_EFF_STANCE_PRONE)
        {
            scale = Movement::PlayerProneSpeedScale.get<float>();
        }

        else if (stance == Game::PM_EFF_STANCE_DUCKED)
        {
            scale = Movement::PlayerDuckedSpeedScale.get<float>();
        }

        else if (stance == Game::PM_EFF_STANCE_LASTSTANDCRAWL)
        {
            scale = Movement::PlayerLastStandCrawlSpeedScale.get<float>();
        }

        return scale;
    }

    __declspec(naked) void Movement::PM_CmdScaleForStanceStub()
    {
        __asm
        {
            pushad

            push edx
            call Movement::PM_CmdScaleForStance
            add esp, 4

            popad
            ret
        }
    }

    float Movement::PM_MoveScale(Game::playerState_s* ps, float forwardmove,
        float rightmove, float upmove)
    {
        assert(ps != nullptr);

        auto max = (std::fabsf(forwardmove) < std::fabsf(rightmove))
            ? std::fabsf(rightmove)
            : std::fabsf(forwardmove);

        if (std::fabsf(upmove) > max)
        {
            max = std::fabsf(upmove);
        }

        if (max == 0.0f)
        {
            return 0.0f;
        }   

        auto total = std::sqrtf(forwardmove * forwardmove
            + rightmove * rightmove + upmove * upmove);
        auto scale = (ps->speed * max) / (127.0f * total);

        if (ps->pm_flags & Game::PMF_WALKING || ps->leanf != 0.0f)
        {
            scale *= 0.4f;
        }

        if (ps->pm_type == Game::PM_NOCLIP)
        {
            return scale * Movement::CGNoclipScaler.get<float>();
        }

        if (ps->pm_type == Game::PM_UFO)
        {
            return scale * Movement::CGUfoScaler.get<float>();
        }

        if (ps->pm_type == Game::PM_SPECTATOR)
        {
            return scale * Movement::PlayerSpectateSpeedScale.get<float>();
        }

        return scale;
    }

    __declspec(naked) void Movement::PM_MoveScaleStub()
    {
        __asm
        {
            pushad

            push [esp + 0xC + 0x20]
            push [esp + 0xC + 0x20]
            push [esp + 0xC + 0x20]
            push esi
            call Movement::PM_MoveScale
            add esp, 0x10

            popad
            ret
        }
    }

    __declspec(naked) void Movement::PM_StepSlideMoveStub()
    {
        __asm
        {
            // Check the value of BGBounces
            push ecx
            push eax

            mov eax, Movement::BGBounces
            mov ecx, dword ptr [eax + 0x10]
            test ecx, ecx

            pop eax
            pop ecx

            // Do not bounce if BGBounces is 0
            jle noBounce

            // Bounce
            push 0x4B1B34
            retn

        noBounce:
            // Original code
            cmp dword ptr [esp + 0x24], 0
            push 0x4B1B48
            retn
        }
    }

    void Movement::PM_ProjectVelocityStub(const float* velIn, const float* normal, float* velOut)
    {
        const auto lengthSquared2D = velIn[0] * velIn[0] + velIn[1] * velIn[1];

        if (std::fabsf(normal[2]) < 0.001f || lengthSquared2D == 0.0)
        {
            velOut[0] = velIn[0];
            velOut[1] = velIn[1];
            velOut[2] = velIn[2];
            return;
        }

        auto newZ = velIn[0] * normal[0] + velIn[1] * normal[1];
        newZ = -newZ / normal[2];
        const auto lengthScale = std::sqrtf((velIn[2] * velIn[2] + lengthSquared2D)
                / (newZ * newZ + lengthSquared2D));

        if (Movement::BGBouncesAllAngles.get<bool>()
            || (lengthScale < 1.f || newZ < 0.f || velIn[2] > 0.f))
        {
            velOut[0] = velIn[0] * lengthScale;
            velOut[1] = velIn[1] * lengthScale;
            velOut[2] = newZ * lengthScale;
        }
    }

    // Double bounces
    void Movement::Jump_ClearStateHook(Game::playerState_s* ps)
    {
        if (Movement::BGBounces->current.integer == Movement::DOUBLE)
            return;

        Game::Jump_ClearState(ps);
    }

    Game::dvar_t* Movement::Dvar_RegisterLastStandSpeedScale(const char* name, float value,
        float min, float max, int, const char* desc)
    {
        Movement::PlayerLastStandCrawlSpeedScale = Dvar::Register<float>(name, value,
            min, max, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED, desc);

        return Movement::PlayerLastStandCrawlSpeedScale.get<Game::dvar_t*>();
    }

    Game::dvar_t* Movement::Dvar_RegisterSpectateSpeedScale(const char* name, float value,
        float min, float max, int, const char* desc)
    {
        Movement::PlayerSpectateSpeedScale = Dvar::Register<float>(name, value,
            min, max, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED, desc);

        return Movement::PlayerSpectateSpeedScale.get<Game::dvar_t*>();
    }

    Movement::Movement()
    {
        Dvar::OnInit([]
        {
            static const char* bg_bouncesValues[] =
            {
                "disabled",
                "enabled",
                "double",
                nullptr
            };

            Movement::PlayerDuckedSpeedScale = Dvar::Register<float>("player_duckedSpeedScale",
                0.65f, 0.0f, 5.0f, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED,
                "The scale applied to the player speed when ducking");

            Movement::PlayerProneSpeedScale = Dvar::Register<float>("player_proneSpeedScale",
                0.15f, 0.0f, 5.0f, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED,
                "The scale applied to the player speed when crawling");

            // 3arch naming convention
            Movement::CGUfoScaler = Dvar::Register<float>("cg_ufo_scaler",
                6.0f, 0.001f, 1000.0f, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED,
                "The speed at which ufo camera moves");

            Movement::CGNoclipScaler = Dvar::Register<float>("cg_noclip_scaler",
                3.0f, 0.001f, 1000.0f, Game::DVAR_FLAG_CHEAT | Game::DVAR_FLAG_REPLICATED,
                "The speed at which noclip camera moves");

            Movement::BGBounces = Game::Dvar_RegisterEnum("bg_bounces",
                bg_bouncesValues, Movement::DISABLED, Game::DVAR_FLAG_REPLICATED, "Bounce glitch settings");

            Movement::BGBouncesAllAngles = Dvar::Register<bool>("bg_bouncesAllAngles",
                false, Game::DVAR_FLAG_REPLICATED, "Force bounce from all angles");
        });

        // Hook PM_CmdScaleForStance in PM_CmdScale_Walk
        Utils::Hook(0x572F34, Movement::PM_CmdScaleForStanceStub, HOOK_CALL).install()->quick();

        //Hook PM_CmdScaleForStance in PM_GetMaxSpeed
        Utils::Hook(0x57395F, Movement::PM_CmdScaleForStanceStub, HOOK_CALL).install()->quick();

        // Hook Dvar_RegisterFloat. Only thing that's changed is that the 0x80 flag is not used.
        Utils::Hook(0x448B66, Movement::Dvar_RegisterLastStandSpeedScale, HOOK_CALL).install()->quick();

        // Hook Dvar_RegisterFloat. Only thing that's changed is that the 0x80 flag is not used.
        Utils::Hook(0x448990, Movement::Dvar_RegisterSpectateSpeedScale, HOOK_CALL).install()->quick();

        // Hook PM_MoveScale so we can add custom speed scale for Ufo and Noclip
        Utils::Hook(0x56F845, Movement::PM_MoveScaleStub, HOOK_CALL).install()->quick();
        Utils::Hook(0x56FABD, Movement::PM_MoveScaleStub, HOOK_CALL).install()->quick();

        // Bounce logic
        Utils::Hook(0x4B1B2D, Movement::PM_StepSlideMoveStub, HOOK_JUMP).install()->quick();
        Utils::Hook(0x57383E, Movement::Jump_ClearStateHook, HOOK_CALL).install()->quick();
        Utils::Hook(0x4B1B97, Movement::PM_ProjectVelocityStub, HOOK_CALL).install()->quick();
    }

    Movement::~Movement()
    {
    }
}
