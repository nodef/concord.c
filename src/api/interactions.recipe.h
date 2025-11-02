PUBLIC(enum, discord_interaction_types, 5, (
        (DISCORD_INTERACTION_PING, =, 1),
        (DISCORD_INTERACTION_APPLICATION_COMMAND, =, 2),
        (DISCORD_INTERACTION_MESSAGE_COMPONENT, =, 3),
        (DISCORD_INTERACTION_APPLICATION_COMMAND_AUTOCOMPLETE, =, 4),
        (DISCORD_INTERACTION_MODAL_SUBMIT, =, 5)
    )
)

PUBLIC(enum, discord_interaction_callback_types, 7, (
        (DISCORD_INTERACTION_PONG, =, 1),
        (DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE, =, 4),
        (DISCORD_INTERACTION_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE, =, 5),
        (DISCORD_INTERACTION_DEFERRED_UPDATE_MESSAGE, =, 6),
        (DISCORD_INTERACTION_UPDATE_MESSAGE, =, 7),
        (DISCORD_INTERACTION_APPLICATION_COMMAND_AUTOCOMPLETE_RESULT, =, 8),
        (DISCORD_INTERACTION_MODAL, =, 9)
    )
)

PUBLIC(struct, discord_interaction, 13, (
        (_, _, u64snowflake, _, id, _, 0ul),
        (_, _, u64snowflake, _, application_id, _, 0ul),
        (_, enum, discord_interaction_types, _, type, _, 0ul),
        (_, struct, discord_interaction_data, *, data, _, 0ul),
        (_, _, u64snowflake, _, guild_id, _, 0ul),
        (_, _, u64snowflake, _, channel_id, _, 0ul),
        (_, struct, discord_guild_member, *, member, _, 0ul),
        (_, struct, discord_user, *, user, _, 0ul),
        (_, _, char, *, token, _, 0ul),
        (_, _, int, _, version, _, 0ul),
        (_, struct, discord_message, *, message, _, 0ul),
        (_, _, char, *, locale, _, 0ul),
        (_, _, char, *, guild_locale, _, 0ul)
    )
)

PRIVATE(struct, discord_interaction_data, 10, (
        (_, _, u64snowflake, _, id, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, enum, discord_application_command_types, _, type, _, 0ul),
        (_, struct, discord_resolved_data, *, resolved, _, 0ul),
        (_, struct, discord_application_command_interaction_data_options, *, options, _, 0ul),
        (_, _, char, *, custom_id, _, 0ul),
        (_, enum, discord_component_types, _, component_type, _, 0ul),
        (_, struct, strings, *, values, _, 0ul),
        (_, _, u64snowflake, _, target_id, _, 0ul),
        (_, struct, discord_components, *, components, _, 0ul)
    )
)

PRIVATE(struct, discord_resolved_data, 6, (
        (_, _, json_char, *, users, _, 0ul),
        (_, _, json_char, *, members, _, 0ul),
        (_, _, json_char, *, roles, _, 0ul),
        (_, _, json_char, *, channels, _, 0ul),
        (_, _, json_char, *, messages, _, 0ul),
        (_, _, json_char, *, attachments, _, 0ul)
    )
)

PRIVATE(struct, discord_message_interaction, 5, (
        (_, _, u64snowflake, _, id, _, 0ul),
        (_, enum, discord_interaction_types, _, type, _, 0ul),
        (_, _, char, *, name, _, 0ul),
        (_, struct, discord_user, *, user, _, 0ul),
        (_, struct, discord_guild_member, *, member, _, 0ul)
    )
)

PUBLIC(struct, discord_interaction_response, 2, (
        (_, enum, discord_interaction_callback_types, _, type, _, 0ul),
        (_, struct, discord_interaction_callback_data, *, data, _, CONCORD_OPTIONAL)
    )
)

PRIVATE(struct, discord_interaction_callback_data, 9, (
        (_, struct, discord_components, *, components, _, CONCORD_OPTIONAL),
        (_, _, bool, _, tts, _, CONCORD_OPTIONAL),
        (_, _, char, *, content, _, CONCORD_OPTIONAL),
        (_, struct, discord_embeds, *, embeds, _, CONCORD_OPTIONAL),
        (_, _, u64bitmask, _, flags, _, CONCORD_OPTIONAL),
        (_, struct, discord_attachments, *, attachments, _, CONCORD_OPTIONAL),
        (_, struct, discord_application_command_option_choices, *, choices, _, 0ul),
        (_, _, char, *, custom_id, _, 0ul),
        (_, _, char, *, title, _, 0ul)
    )
)

PUBLIC(struct, discord_edit_original_interaction_response, 6, (
        (_, _, u64snowflake, _, thread_id, _, 0ul),
        (_, _, char, *, content, _, 0ul),
        (_, struct, discord_embeds, *, embeds, _, CONCORD_OPTIONAL),
        (_, struct, discord_allowed_mention, *, allowed_mentions, _, CONCORD_OPTIONAL),
        (_, struct, discord_components, *, components, _, CONCORD_OPTIONAL),
        (_, struct, discord_attachments, *, attachments, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_create_followup_message, 9, (
        (_, _, bool, _, wait, _, 0ul),
        (_, _, u64snowflake, _, thread_id, _, 0ul),
        (_, _, char, *, avatar_url, _, 0ul),
        (_, _, bool, _, tts, _, 0ul),
        (_, struct, discord_embeds, *, embeds, _, CONCORD_OPTIONAL),
        (_, struct, discord_allowed_mention, *, allowed_mentions, _, CONCORD_OPTIONAL),
        (_, struct, discord_components, *, components, _, CONCORD_OPTIONAL),
        (_, struct, discord_attachments, *, attachments, _, CONCORD_OPTIONAL),
        (_, _, u64bitmask, _, flags, _, CONCORD_OPTIONAL)
    )
)

PUBLIC(struct, discord_edit_followup_message, 6, (
        (_, _, u64snowflake, _, thread_id, _, 0ul),
        (_, _, char, *, content, _, 0ul),
        (_, struct, discord_embeds, *, embeds, _, CONCORD_OPTIONAL),
        (_, struct, discord_allowed_mention, *, allowed_mentions, _, CONCORD_OPTIONAL),
        (_, struct, discord_components, *, components, _, CONCORD_OPTIONAL),
        (_, struct, discord_attachments, *, attachments, _, CONCORD_OPTIONAL)
    )
)
