PUBLIC(struct, discord_voice_state, 13, (
        (_, _, u64snowflake, _, guild_id, _, 0ul),
        (_, _, u64snowflake, _, channel_id, _, 0ul),
        (_, _, u64snowflake, _, user_id, _, 0ul),
        (_, struct, discord_guild_member, *, member, _, 0ul),
        (_, _, char, *, session_id, _, 0ul),
        (_, _, bool, _, deaf, _, 0ul),
        (_, _, bool, _, mute, _, 0ul),
        (_, _, bool, _, self_deaf, _, 0ul),
        (_, _, bool, _, self_mute, _, 0ul),
        (_, _, bool, _, self_stream, _, 0ul),
        (_, _, bool, _, self_video, _, 0ul),
        (_, _, bool, _, suppress, _, 0ul),
        (_, _, u64unix_ms, _, request_to_speak_timestamp, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_voice_states, 3, (
        (_, _, int, _, size, _, 0ul),
        (_, struct, discord_voice_state, *, array, _, 0ul),
        (_, _, int, _, realsize, _, 0ul)
    )
)

PRIVATE(struct, discord_voice_region, 5, (
        (_, _, char, *, id, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, _, bool, _, optimal, _, 0ul),
        (_, _, bool, _, deprecated, _, 0ul),
        (_, _, bool, _, custom, _, 0ul)
    )
)

PUBLIC(struct, discord_voice_regions, 3, (
        (_, _, int, _, size, _, 0ul),
        (_, struct, discord_voice_region, *, array, _, 0ul),
        (_, _, int, _, realsize, _, 0ul)
    )
)
