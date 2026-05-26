#pragma once

enum class CardTiming
{
    BEFORE_OWN_TURN,
    AS_OWN_TURN,
    AFTER_OWN_TURN,
    AFTER_OPPONENT_TURN,
    ANYTIME
};

enum class CardTargetRequirement
{
    NONE,
    ENEMY_PAWN,
    FRIENDLY_KNIGHT,
    FRIENDLY_PAWNS

};

enum class CardEffectType
{
    NONE,
    DESTROY_TARGET,
    CHARGE_KNIGHT,
    MOVE_PAWNS
};
