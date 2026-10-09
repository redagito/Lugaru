#pragma once

enum anim_attack_type
{
    neutral,
    normalattack,
    reversed,
    reversal
};

enum anim_height_type
{
    lowheight,
    middleheight,
    highheight
};

enum animation_type
{
#define DECLARE_ANIM(id, ...) id,
#include "Animation.inc"
#undef DECLARE_ANIM
    animation_count
};

enum animation_bit_offsets
{
#define DECLARE_ANIM_BIT(bit) o_##bit,
#include "Animation.inc"
#undef DECLARE_ANIM_BIT
    animation_bit_count
};

enum animation_bits_def
{
#define DECLARE_ANIM_BIT(bit) bit = 1 << o_##bit,
#include "Animation.inc"
#undef DECLARE_ANIM_BIT
};

static const int animation_bits[animation_count] = {
#define DECLARE_ANIM(id, name, height, type, bits) bits,
#include "Animation.inc"
#undef DECLARE_ANIM
};