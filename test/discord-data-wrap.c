#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "greatest.h"
#include "discord.h"

struct test_client {
    struct reflectc *registry;
    struct discord *client;
};

static struct test_client
create_test_client(void)
{
    struct test_client tc = { 0 };
    tc.registry = reflectc_init();
    tc.client = (struct discord *)tc.registry;
    return tc;
}

static void
dispose_test_client(struct test_client *tc)
{
    if (!tc || !tc->registry) return;
    reflectc_dispose(tc->registry);
    tc->registry = NULL;
    tc->client = NULL;
}

TEST
to_json_roundtrip_components(void)
{
    struct test_client tc = create_test_client();
    ASSERT_NEQ(NULL, tc.registry);

    struct discord_component button = {
        .type = DISCORD_COMPONENT_BUTTON,
        .style = DISCORD_BUTTON_PRIMARY,
        .custom_id = "action_button",
        .label = "Click me",
    };
    struct discord_component button_array[] = { button };
    struct discord_components button_children = {
        .size = (int)(sizeof(button_array) / sizeof *button_array),
        .array = button_array,
        .realsize = (int)(sizeof(button_array) / sizeof *button_array),
    };

    struct discord_component row = {
        .type = DISCORD_COMPONENT_ACTION_ROW,
        .components = &button_children,
    };
    struct discord_component row_array[] = { row };
    struct discord_components rows = {
        .size = (int)(sizeof(row_array) / sizeof *row_array),
        .array = row_array,
        .realsize = (int)(sizeof(row_array) / sizeof *row_array),
    };

    struct discord_create_message original = {
        .content = "Testing components",
        .components = &rows,
        .flags = DISCORD_MESSAGE_EPHEMERAL,
    };

    original.components = &rows;

    printf(
        "original.components before JSON = %p (expected %p), field addr=%p\n",
        (void *)original.components, (void *)&rows,
        (void *)&original.components);
    fflush(stdout);

    char *json = NULL;
    size_t json_cap = 0;
    CCORDcode code = discord_data_to_json(
        struct discord_create_message, tc.client, &original, &json, &json_cap);
    if (code != CCORD_OK) {
        FAILm(discord_code_as_string(code));
    }
    ASSERT_NEQ(NULL, json);
    ASSERT(json_cap > 0);

    ASSERT(strstr(json, "\"components\"") != NULL);
    ASSERT(strstr(json, "\"custom_id\":\"action_button\"") != NULL);
    ASSERT(strstr(json, "\"flags\":") != NULL);

    size_t json_len = strlen(json);
    struct discord_create_message parsed = { 0 };
    ASSERT_EQ(CCORD_OK,
              discord_data_from_json(struct discord_create_message, tc.client,
                                     json, json_len, &parsed));
    ASSERT_NEQ(NULL, parsed.content);
    ASSERT_STR_EQ(original.content, parsed.content);
    printf("json output: %s\n", json);
    ASSERT_NEQ(NULL, parsed.components);
    ASSERT_EQ(1, parsed.components->size);
    ASSERT_NEQ(NULL, parsed.components->array);
    ASSERT_EQ(DISCORD_COMPONENT_ACTION_ROW, parsed.components->array[0].type);
    ASSERT(parsed.components->array[0].components != NULL);
    ASSERT_EQ(1, parsed.components->array[0].components->size);
    ASSERT(parsed.components->array[0].components->array != NULL);
    ASSERT_EQ(DISCORD_COMPONENT_BUTTON,
              parsed.components->array[0].components->array[0].type);
    ASSERT_EQ(DISCORD_BUTTON_PRIMARY,
              parsed.components->array[0].components->array[0].style);
    ASSERT(parsed.components->array[0].components->array[0].label != NULL);
    ASSERT_STR_EQ("Click me",
                  parsed.components->array[0].components->array[0].label);
    ASSERT(parsed.components->array[0].components->array[0].custom_id != NULL);
    ASSERT_STR_EQ("action_button",
                  parsed.components->array[0].components->array[0].custom_id);
    ASSERT_EQ(DISCORD_MESSAGE_EPHEMERAL, parsed.flags);

    discord_data_cleanup(tc.client, &parsed);
    free(json);
    dispose_test_client(&tc);
    PASS();
}

TEST
strings_roundtrip(void)
{
    struct test_client tc = create_test_client();
    ASSERT(tc.registry != NULL);

    char *values_array[] = { "foo", "bar", "baz" };
    struct strings values = {
        .size = (int)(sizeof(values_array) / sizeof *values_array),
        /* generated codec stores a char** behind a char* field */
        .array = (char *)values_array,
        .realsize = (int)(sizeof(values_array) / sizeof *values_array),
    };

    char *json = NULL;
    size_t json_cap = 0;
    CCORDcode code = discord_data_to_json(struct strings, tc.client, &values,
                                          &json, &json_cap);
    if (code != CCORD_OK) {
        FAILm(discord_code_as_string(code));
    }
    ASSERT(json != NULL);
    ASSERT_EQ(strlen("[\"foo\",\"bar\",\"baz\"]"), strlen(json));
    ASSERT_STR_EQ("[\"foo\",\"bar\",\"baz\"]", json);

    struct strings parsed = { 0 };
    CCORDcode parse_code = discord_data_from_json(struct strings, tc.client,
                                                  json, strlen(json), &parsed);
    if (parse_code != CCORD_OK) {
        FAILm(discord_code_as_string(parse_code));
    }
    ASSERT_EQ(values.size, parsed.size);
    ASSERT(parsed.array != NULL);
    char **parsed_values = (char **)parsed.array;
    ASSERT_STR_EQ("foo", parsed_values[0]);
    ASSERT_STR_EQ("bar", parsed_values[1]);
    ASSERT_STR_EQ("baz", parsed_values[2]);

    discord_data_cleanup(tc.client, &parsed);
    free(json);
    dispose_test_client(&tc);
    PASS();
}

TEST
optional_bool_serialization(void)
{
    struct test_client tc = create_test_client();
    ASSERT(tc.registry != NULL);

    struct discord_create_message message = {
        .content = "No TTS",
    };

    char *json = NULL;
    size_t json_cap = 0;
    CCORDcode code = discord_data_to_json(
        struct discord_create_message, tc.client, &message, &json, &json_cap);
    if (code != CCORD_OK) {
        FAILm(discord_code_as_string(code));
    }
    ASSERT(json != NULL);
    ASSERT(strstr(json, "\"tts\"") == NULL);
    free(json);

    message.tts = true;
    json = NULL;
    json_cap = 0;
    code = discord_data_to_json(struct discord_create_message, tc.client,
                                &message, &json, &json_cap);
    if (code != CCORD_OK) {
        FAILm(discord_code_as_string(code));
    }
    ASSERT(strstr(json, "\"tts\":true") != NULL);

    free(json);
    dispose_test_client(&tc);
    PASS();
}

SUITE(data_wrap)
{
    RUN_TEST(to_json_roundtrip_components);
    RUN_TEST(strings_roundtrip);
    RUN_TEST(optional_bool_serialization);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    GREATEST_MAIN_BEGIN();
    RUN_SUITE(data_wrap);
    GREATEST_MAIN_END();
}
