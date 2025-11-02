PUBLIC(enum, discord_guild_scheduled_event_privacy_level, 1, (
        (DISCORD_GUILD_SCHEDULED_EVENT_GUILD_ONLY, =, 2)
    )
)

PUBLIC(enum, discord_guild_scheduled_event_entity_types, 3, (
        (DISCORD_GUILD_SCHEDULED_EVENT_ENTITY_STAGE_INSTANCE, =, 1),
        (DISCORD_GUILD_SCHEDULED_EVENT_ENTITY_VOICE, =, 2),
        (DISCORD_GUILD_SCHEDULED_EVENT_ENTITY_EXTERNAL, =, 3)
    )
)

PUBLIC(enum, discord_guild_scheduled_event_status, 4, (
        (DISCORD_GUILD_SCHEDULED_EVENT_SCHEDULED, =, 1),
        (DISCORD_GUILD_SCHEDULED_EVENT_ACTIVE, =, 2),
        (DISCORD_GUILD_SCHEDULED_EVENT_COMPLETED, =, 3),
        (DISCORD_GUILD_SCHEDULED_EVENT_CANCELED, =, 4)
    )
)

PUBLIC(struct, discord_guild_scheduled_event, 16, (
        (_, _, u64snowflake, _, id, _, 0ul),
        (_, _, u64snowflake, _, guild_id, _, 0ul),
        (_, _, u64snowflake, _, channel_id, _, 0ul),
        (_, _, u64snowflake, _, creator_id, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, description, _, 0ul),
        (_, _, u64unix_ms, _, scheduled_start_time, _, 0ul),
        (_, _, u64unix_ms, _, scheduled_end_time, _, 0ul),
        (_, enum, discord_guild_scheduled_event_privacy_level, _, privacy_level, _, CONCORD_OPTIONAL),
        (_, enum, discord_guild_scheduled_event_status, _, status, _, CONCORD_OPTIONAL),
        (_, enum, discord_guild_scheduled_event_entity_types, _, entity_type, _, CONCORD_OPTIONAL),
        (_, _, u64snowflake, _, entity_id, _, 0ul),
        (_, struct, discord_guild_scheduled_event_entity_metadata, *, entity_metadata, _, CONCORD_OPTIONAL),
        (_, struct, discord_user, *, creator, _, CONCORD_OPTIONAL),
        (_, _, int, _, user_count, _, 0ul),
        (_, _, char, *, image, _, 0ul)
    )
)

PRIVATE(struct, discord_guild_scheduled_event_entity_metadata, 1, (
        (_, _, char, *, location, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_guild_scheduled_events, 3, (
        (_, _, int, _, size, _, 0ul),
        (_, struct, discord_guild_scheduled_event, *, array, _, 0ul),
        (_, _, int, _, realsize, _, 0ul)
    )
)

PRIVATE(struct, discord_guild_scheduled_event_user, 3, (
        (_, _, u64snowflake, _, guild_scheduled_event_id, _, 0ul),
        (_, struct, discord_user, *, user, _, CONCORD_OPTIONAL),
        (_, struct, discord_guild_member, *, member, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_guild_scheduled_event_users, 3, (
        (_, _, int, _, size, _, 0ul),
        (_, struct, discord_guild_scheduled_event_user, *, array, _, 0ul),
        (_, _, int, _, realsize, _, 0ul)
    )
)

PRIVATE(struct, discord_list_guild_scheduled_events, 1, (
        (_, _, bool, _, with_user_count, _, 0ul)
    )
)

PUBLIC(struct, discord_create_guild_scheduled_event, 10, (
        (_, _, char, *, reason, _, 0ul),
        (_, _, u64snowflake, _, channel_id, _, CONCORD_OPTIONAL),
        (_, struct, discord_guild_scheduled_event_entity_metadata, *, entity_metadata, _, CONCORD_OPTIONAL),
        (_, _, char, *, name, _, 0ul),
        (_, enum, discord_guild_scheduled_event_privacy_level, _, privacy_level, _, CONCORD_OPTIONAL),
        (_, _, u64unix_ms, _, scheduled_start_time, _, CONCORD_OPTIONAL),
        (_, _, u64unix_ms, _, scheduled_end_time, _, CONCORD_OPTIONAL),
        (_, _, char, *, description, _, CONCORD_OPTIONAL),
        (_, enum, discord_guild_scheduled_event_entity_types, _, entity_type, _, CONCORD_OPTIONAL),
        (_, _, char, *, image, _, CONCORD_OPTIONAL)
    )
)

PRIVATE(struct, discord_get_guild_scheduled_event, 1, (
        (_, _, bool, _, with_user_count, _, 0ul)
    )
)

PUBLIC(struct, discord_modify_guild_scheduled_event, 10, (
        (_, _, char, *, reason, _, 0ul),
        (_, _, u64snowflake, _, channel_id, _, CONCORD_OPTIONAL),
        (_, struct, discord_guild_scheduled_event_entity_metadata, *, entity_metadata, _, CONCORD_OPTIONAL),
        (_, _, char, *, name, _, 0ul),
        (_, _, u64unix_ms, _, scheduled_start_time, _, CONCORD_OPTIONAL),
        (_, _, u64unix_ms, _, scheduled_end_time, _, CONCORD_OPTIONAL),
        (_, _, char, *, description, _, CONCORD_OPTIONAL),
        (_, enum, discord_guild_scheduled_event_entity_types, _, entity_type, _, CONCORD_OPTIONAL),
        (_, enum, discord_guild_scheduled_event_status, _, status, _, CONCORD_OPTIONAL),
        (_, _, char, *, image, _, CONCORD_OPTIONAL)
    )
)

PRIVATE(struct, discord_get_guild_scheduled_event_users, 4, (
        (_, _, int, _, limit, _, 0ul),
        (_, _, bool, _, with_member, _, 0ul),
        (_, _, u64snowflake, _, before, _, CONCORD_OPTIONAL),
        (_, _, u64snowflake, _, after, _, CONCORD_OPTIONAL)
    )
)
