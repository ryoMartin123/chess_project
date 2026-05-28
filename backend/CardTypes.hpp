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
    FRIENDLY_NON_KING,
    FRIENDLY_KNIGHT,
    FRIENDLY_PAWNS,
    ENEMY_NON_KING_QUEEN,
    FRIENDLY_KING
};

enum class CardEffectType
{
    NONE,
    DESTROY_TARGET,
    CHARGE_KNIGHT,
    MOVE_PAWNS,
    CANCEL_LAST_ACTION,
    BOG_MOVE,
    APPLY_NEUTRALITY,
    APPLY_WARLORD
};
