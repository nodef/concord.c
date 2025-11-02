PUBLIC(struct, discord_guild_template, 11, (
        (_, _, char, *, code, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, description, _, 0ul),
        (_, _, int, _, usage_count, _, 0ul),
        (_, _, u64snowflake, _, creator_id, _, 0ul),
        (_, struct, discord_user, *, creator, _, 0ul),
        (_, _, u64unix_ms, _, created_at, _, 0ul),
        (_, _, u64unix_ms, _, updated_at, _, 0ul),
        (_, _, u64snowflake, _, source_guild_id, _, 0ul),
        (_, struct, discord_guild, *, serialized_source_guild, _, 0ul),
        (_, _, bool, _, is_dirty, _, 0ul)
    )
)

PUBLIC(struct, discord_guild_templates, 3, (
        (_, _, int, _, size, _, 0ul),
        (_, struct, discord_guild_template, *, array, _, 0ul),
        (_, _, int, _, realsize, _, 0ul)
    )
)

PUBLIC(struct, discord_create_guild_from_guild_template, 2, (
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, icon, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_create_guild_template, 2, (
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, description, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_modify_guild_template, 2, (
        (_, _, char, *, name, _, CONCORD_OPTIONAL),
        (_, _, char, *, description, _, CONCORD_OPTIONAL)
    )
)
