#pragma once
#include "sigs.hpp"
#include <core/utils.h>
#include <hooks/hooks.h>
#include <sdk/player/NetAvatar.h>

std::vector<uintptr_t> sigs::database{};


uintptr_t sigs::add_pattern(std::string name, std::string pattern, int type, int offset, bool ignore) {
    uintptr_t address = 0;
    switch (type) {
        case sig::type::direct: address = utils::find_pattern(pattern.c_str(), false, offset); break;
        case sig::type::fstart: address = utils::find_func_start(pattern.c_str()) + offset; break;
        case sig::type::call: address = detail::get_call(pattern.c_str(), offset); break;
    }

    if (address == 0 || address <= offset || address >= 0xffffffffffffefff) {
        if (ignore)
            return 0;
        utils::printc("91", "pattern of %s not found!", name.c_str());
        address = 0;
    }

    database.push_back(address);
    return address;
}
void add_pattern2() {
}
void sigs::init() {
   
    auto bp = add_pattern("gt.cpp/patch_banbypass", "00 3B C1 75 ? 85 C9", sig::type::direct, 3, true);
    if (!bp) //will be invalid if ban bypass already patched, so we are doing this.
        add_pattern("gt.cpp/patch_banbypass", "00 3B C1 90 90 85 C9", sig::type::direct, 3);

    add_pattern("sdk.cpp/GetGameLogic", "E8 ? ? ? ? 8b 17 ? 8d 88", sig::type::call);
    add_pattern("WorldCamera.h/WorldToScreen", "00 e8 ? ? ? ? 49 8b ? ? 41 ? 00 04", sig::type::call, 1);
    add_pattern("sdk.cpp/GetClient", "E8 ? ? ? ? 45 ? ? 89 ? ? ? 48 8D ? ? 48", sig::type::call);

   
    //all hooks.cpp
    hookmgr->add_hook("BaseApp::SetFPSLimit", "00 00 0F 57 C0 0F 2F C8 72", sig::type::fstart, hooks::BaseApp_SetFPSLimit);
    hookmgr->add_hook("LogMsg", "00 28 00 00 45", sig::type::fstart, hooks::LogMsg);
    hookmgr->add_hook("NetAvatar::CanMessageT4", "48 8b ce e8 ? ? ? ? 84 c0 74 ? e8", sig::type::call, hooks::NetAvatar_CanMessageT4, 3);
    hookmgr->add_hook("SendPacketRaw", "00 81 FE 40 42 0F 00", sig::type::fstart, hooks::SendPacketRaw);
    hookmgr->add_hook("HandleTouch", "83 B8 ?? ?? ?? ?? 12 75", sig::type::fstart,hooks::HandleTouch);
    hookmgr->add_hook("WorldCamera_OnUpdate", "89 43 10 0f 2f", sig::type::fstart, hooks::WorldCamera_OnUpdate);
    hookmgr->add_hook("SendPacket", "02 00 00 00 e8 ? ? ? ? 90 48 8d 4c 24 50", sig::type::call, hooks::SendPacket, 4);
    hookmgr->add_hook("ProcessTankUpdatePacket", "83 78 04 71 75 ? 49", sig::type::fstart, hooks::ProcessTankUpdatePacket);
    hookmgr->add_hook("CanSeeGhosts", "04 00 00 00 e8 ? ? ? ? 8b c8 e8", sig::type::call, hooks::CanSeeGhosts, 11);
    hookmgr->add_hook("NetAvatar::Gravity", "B9 CA 27 00 00", sig::type::fstart, hooks::NetAvatar_Gravity);
    hookmgr->add_hook("NetHTTP::Update", "05 4D 01 00 00 89 ?? ?? 00", sig::type::fstart, hooks::NetHTTP_Update);
    hookmgr->add_hook("NetAvatar::ProcessAcceleration", "83 78 04 3c", sig::type::fstart, hooks::ProcessAcceleration);
    hookmgr->add_hook("App::Update", "00 E8 ? ? ? ? C6 ?? ? ? 00 00 01 B9 ? ? 00 00", sig::type::fstart, hooks::App_Update);

    size_t invalid = 0;
    for (auto sig : database) {
        if (!sig)
            invalid++;
    }

    if (invalid == 0)
        utils::printc("92", "%zd signatures in db, all valid\n", database.size());
}
