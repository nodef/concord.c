PUBLIC(enum, discord_webhook_types, 3, (
        (DISCORD_WEBHOOK_INCOMING, =, 1),
        (DISCORD_WEBHOOK_CHANNEL_FOLLOWER, =, 2),
        (DISCORD_WEBHOOK_APPLICATION, =, 3)
    )
)

PUBLIC(struct, discord_webhook, 10, (
        (_, _, u64snowflake, _, id, _, 0ul),
        (_, enum, discord_webhook_types, _, type, _, CONCORD_OPTIONAL),
        (_, _, u64snowflake, _, guild_id, _, 0ul),
        (_, _, u64snowflake, _, channel_id, _, 0ul),
        (_, struct, discord_user, *, user, _, CONCORD_OPTIONAL),
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, token, _, 0ul),
        (_, _, u64snowflake, _, application_id, _, 0ul),
        (_, struct, discord_channel, *, source_channel, _, CONCORD_OPTIONAL),
        (_, _, char, *, url, _, 0ul)
    )
)

PUBLIC(struct, discord_webhooks, 3, (
        (_, _, int, _, size, _, 0ul),
        (_, struct, discord_webhook, *, array, _, 0ul),
        (_, _, int, _, realsize, _, 0ul)
    )
)

PUBLIC(struct, discord_create_webhook, 3, (
        (_, _, char, *, reason, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, avatar, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_modify_webhook, 4, (
        (_, _, char, *, reason, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, avatar, _, CONCORD_OPTIONAL),
        (_, _, u64snowflake, _, channel_id, _, 0ul)
    )
)

PRIVATE(struct, discord_delete_webhook, 1, (
        (_, _, char, *, reason, _, 0ul)
    )
)

PUBLIC(struct, discord_modify_webhook_with_token, 2, (
        (_, _, char, *, name, _, 0ul),
        (_, _, char, *, avatar, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_execute_webhook, 11, (
        (_, _, bool, _, wait, _, 0ul),
        (_, _, u64snowflake, _, thread_id, _, 0ul),
        (_, _, char, *, content, _, 0ul),
        (_, _, char, *, username, _, 0ul),
        (_, _, char, *, avatar_url, _, 0ul),
        (_, _, bool, _, tts, _, 0ul),
        (_, struct, discord_embeds, *, embeds, _, CONCORD_OPTIONAL),
        (_, struct, discord_allowed_mention, *, allowed_mentions, _, CONCORD_OPTIONAL),
        (_, struct, discord_components, *, components, _, CONCORD_OPTIONAL),
        (_, struct, discord_attachments, *, attachments, _, CONCORD_OPTIONAL),
        (_, _, u64bitmask, _, flags, _, CONCORD_OPTIONAL)
    )
)

PRIVATE(struct, discord_get_webhook_message, 1, (
        (_, _, u64snowflake, _, thread_id, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_edit_webhook_message, 6, (
        (_, _, u64snowflake, _, thread_id, _, 0ul),
        (_, _, char, *, content, _, 0ul),
        (_, struct, discord_embeds, *, embeds, _, CONCORD_OPTIONAL),
        (_, struct, discord_allowed_mention, *, allowed_mentions, _, CONCORD_OPTIONAL),
        (_, struct, discord_components, *, components, _, CONCORD_OPTIONAL),
        (_, struct, discord_attachments, *, attachments, _, CONCORD_OPTIONAL)
    )
)

PRIVATE(struct, discord_delete_webhook_message, 1, (
        (_, _, u64snowflake, _, thread_id, _, CONCORD_OPTIONAL)
    )
)
