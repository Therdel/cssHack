#include "GameVars.hpp"
#include "libNames.hpp"
#include "overlay_structs.hpp"
#include "../MemoryScanner/MemoryScanner.hpp"
#include "../MemoryUtils.hpp"
#include "Signatures.hpp"

template <typename T>
auto read_reference_from_signature(const SignatureAOI &signatureAOI) -> T& {
    uintptr_t scan_result = MemoryScanner::scanSignatureExpectOneResult(signatureAOI);
    uintptr_t* pointer_to_address_in_code = reinterpret_cast<uintptr_t*>(scan_result);
    uintptr_t address_in_code = *pointer_to_address_in_code;
    T& reference = *reinterpret_cast<T*>(address_in_code);
    return reference;
}

template <typename T>
auto read_reference_from_static_mem(const std::string_view libName,
				                    std::span<const ptrdiff_t> offsets,
                                    OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET) -> T& {
    uintptr_t library_base = MemoryUtils::lib_base_32(libName);
    uintptr_t pointer = library_base;
    for (size_t i=0; i<offsets.size(); ++i) {
        const bool last_offset = i == offsets.size()-1;
        const ptrdiff_t offset = offsets[i];

        const OffsetType offset_type = last_offset ? lastOffsetType : OffsetType::DEREFERENCE;
        if (offset_type == OffsetType::DEREFERENCE) {
            pointer += offset;
            pointer = *reinterpret_cast<uintptr_t*>(pointer);
        } else {
            pointer += offset;
        }
    }

    T& reference = *reinterpret_cast<T*>(pointer);
    return reference;
}

GameVars::GameVars()
    : on_ground{read_reference_from_signature<uint32_t>(Signatures::onGround)}
    , do_jump{read_reference_from_signature<uint32_t>(Signatures::doJump)}
    , do_attack_1{read_reference_from_signature<uint32_t>(Signatures::doAttack)}
    , player_pos{read_reference_from_signature<glm::vec3>(Signatures::playerPos)}
    , angles{read_reference_from_signature<glm::vec3>(Signatures::aimAngles)}
    , angles_visual{read_reference_from_static_mem<glm::vec3>(libNames::client, {&Offsets::client_viewAngleVis, 1})}
    , is_ingame{read_reference_from_static_mem<uint8_t>(libNames::materialsystem, {&Offsets::materialsystem_isIngame, 1})}
    , is_inmenu{read_reference_from_static_mem<uint32_t>(libNames::vgui2, {&Offsets::vgui2_isInMenu, 1})}
    , radar_struct{read_reference_from_signature<overlay_structs::RadarStruct>(Signatures::playerArray_base)}
    , localplayer_base{read_reference_from_signature<uintptr_t>(Signatures::localplayer_base)}
#ifdef __linux__
    , op_sdl_pollEvent_call{(uintptr_t)read_reference_from_static_mem<uintptr_t>(libNames::launcher, {&Offsets::launcher_sdl_pollEvent_caller, 1})}
#else
    , op_sdl_pollEvent_call{(uintptr_t)read_reference_from_static_mem<uintptr_t>(libNames::inputsystem, {&Offsets::inputsystem_sdl_pollEvent_caller, 1})}
#endif
    , fov_horizontal_degrees{read_reference_from_static_mem<float>(libNames::engine, {&Offsets::engine_fov_horizontal, 1})}
    , mat_viewmodel{read_reference_from_static_mem<glm::mat4>(libNames::client, {&Offsets::client_matViewModel, 1})}
    , screen_dimensions{read_reference_from_static_mem<std::pair<int, int>>(libNames::engine, {&Offsets::engine_screenDimensions, 1})}
{
}

auto GameVars::scan() -> GameVars {
    return {};
}

auto GameVars::localplayer() -> std::optional<overlay_structs::LocalPlayer*> {
    if (&localplayer_base == nullptr) {
        return std::nullopt;
    } else {
        auto* pointer = reinterpret_cast<overlay_structs::LocalPlayer*>(localplayer_base);
        return pointer;
    }
}

auto GameVars::localplayer() const -> std::optional<const overlay_structs::LocalPlayer*> {
    if (&localplayer_base == nullptr) {
        return std::nullopt;
    } else {
        auto* pointer = reinterpret_cast<overlay_structs::LocalPlayer const*>(localplayer_base);
        return pointer;
    }
}