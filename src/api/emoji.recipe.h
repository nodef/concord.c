PUBLIC(struct, discord_emoji, 8, (
        (_, _, u64snowflake, _, id, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, struct, snowflakes, *, roles, _, CONCORD_OPTIONAL),
        (_, struct, discord_user, *, user, _, CONCORD_OPTIONAL),
        (_, _, bool, _, require_colons, _, 0ul),
        (_, _, bool, _, managed, _, 0ul),
        (_, _, bool, _, animated, _, 0ul),
        (_, _, bool, _, available, _, 0ul)
    )
)

PUBLIC(struct, discord_emojis, 3, (
        (_, _, int, _, size, _, 0ul),
        (_, struct, discord_emoji, *, array, _, 0ul),
        (_, _, int, _, realsize, _, 0ul)
    )
)

PUBLIC(struct, discord_create_guild_emoji, 4, (
        (_, _, char, *, reason, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, image, _, 0ul),
        (_, struct, snowflakes, *, roles, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_modify_guild_emoji, 4, (
        (_, _, char, *, reason, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, image, _, 0ul),
        (_, struct, snowflakes, *, roles, _, CONCORD_OPTIONAL)
    )
)

PRIVATE(struct, discord_delete_guild_emoji, 1, (
        (_, _, char, *, reason, _, 0ul)
    )
)
