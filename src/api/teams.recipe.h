PUBLIC(enum, discord_membership_state, 2, (
        (DISCORD_MEMBERSHIP_INVITED, =, 1),
        (DISCORD_MEMBERSHIP_ACCEPTED, =, 2)
    )
)

PUBLIC(struct, discord_team, 5, (
        (_, _, char, *, icon, _, 0ul),
        (_, _, u64snowflake, _, id, _, 0ul),
        (_, struct, discord_team_members, *, members, _, CONCORD_OPTIONAL),
        (_, _, char, *, name, _, 0ul),
        (_, _, u64snowflake, _, owner_user_id, _, 0ul)
    )
)

PRIVATE(struct, discord_team_member, 4, (
        (_, enum, discord_membership_state, _, membership_state, _, 0ul),
        (_, struct, bitmasks, *, permissions, _, CONCORD_OPTIONAL),
        (_, _, u64snowflake, _, team_id, _, 0ul),
        (_, struct, discord_user, *, user, _, CONCORD_OPTIONAL)
    )
)

PRIVATE(struct, discord_team_members, 3, (
        (_, _, int, _, size, _, 0ul),
        (_, struct, discord_team_member, *, array, _, 0ul),
        (_, _, int, _, realsize, _, 0ul)
    )
)
