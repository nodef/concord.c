#!/usr/bin/env python3
"""Convert Concord gencodecs recipes into Reflect-C recipes.

This script parses the existing gencodecs macro DSL (PUB_STRUCT, LIST,
FIELD_*, ENUM, etc.) and emits Reflect-C api files with equivalent
structure/type metadata. By default it writes to ``src/api/`` so the new
Reflect-C DSL stays under version control, ready for generation into
``generated/``. Manual edits to the output are discouraged; rerun the
tool whenever upstream api change.
"""
from __future__ import annotations

import argparse
import dataclasses
import itertools
import os
import pathlib
import re
import sys
from typing import Iterable, Iterator, List, Sequence, Tuple

# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------


DEFAULT_ATTR = "0ul"
OPTIONAL_ATTR = "CONCORD_OPTIONAL"


@dataclasses.dataclass
class Member:
    qualifier: str
    container: str
    type_name: str
    decorator: str
    name: str
    dimensions: str
    attrs: str = DEFAULT_ATTR

    def to_tuple(self) -> str:
        return (
            f"        ({self.qualifier}, {self.container}, {self.type_name}, "
            f"{self.decorator}, {self.name}, {self.dimensions}, {self.attrs})"
        )


@dataclasses.dataclass
class StructDef:
    name: str
    members: List[Member]
    public: bool = True

    @property
    def count(self) -> int:
        return len(self.members)

    def render(self) -> str:
        macro = "PUBLIC" if self.public else "PRIVATE"
        tuples = ",\n".join(m.to_tuple() for m in self.members)
        return (
            f"{macro}(struct, {self.name}, {self.count}, (\n"
            f"{tuples}\n"
            f"    )\n"
            f")"
        )


@dataclasses.dataclass
class EnumEntry:
    name: str
    op: str
    value: str

    def to_tuple(self) -> str:
        return f"        ({self.name}, {self.op}, {self.value})"


@dataclasses.dataclass
class EnumDef:
    name: str
    entries: List[EnumEntry]
    public: bool = True

    @property
    def count(self) -> int:
        return len(self.entries)

    def render(self) -> str:
        macro = "PUBLIC" if self.public else "PRIVATE"
        tuples = ",\n".join(entry.to_tuple() for entry in self.entries)
        return (
            f"{macro}(enum, {self.name}, {self.count}, (\n"
            f"{tuples}\n"
            f"    )\n"
            f")"
        )


@dataclasses.dataclass
class ConstantDef:
    name: str
    value: str

    def render(self) -> str:
        return f"#define {self.name} ({self.value})"


@dataclasses.dataclass
class RecipeFile:
    path: pathlib.Path
    structs: List[StructDef]
    enums: List[EnumDef]
    constants: List[ConstantDef]
    includes: List[str]

    def render(self) -> str:
        lines: List[str] = []
        if self.includes:
            has_body = bool(self.structs or self.enums or self.constants)
            if has_body:
                lines.append("#ifdef REFLECTC_PRE_HOOKS")
                lines.append("/*#!")
                for inc in self.includes:
                    lines.append(f"#include {inc}")
                lines.append("*/")
                lines.append("#endif")
                lines.append("")
            else:
                for inc in self.includes:
                    lines.append(f"#include {inc}")
                if self.includes:
                    lines.append("")
        if self.constants:
            lines.append("#ifdef REFLECTC_PRE_HOOKS")
            lines.append("/*#!")
            for const in self.constants:
                lines.append(const.render())
            lines.append("*/")
            lines.append("#endif")
            lines.append("")
        for enum in self.enums:
            lines.append(enum.render())
            lines.append("")
        for struct in self.structs:
            lines.append(struct.render())
            lines.append("")
        return "\n".join(lines).rstrip() + "\n"


# ---------------------------------------------------------------------------
# Parsing helpers
# ---------------------------------------------------------------------------

_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
_IDENTIFIER_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
_MACRO_START_RE = re.compile(r"^(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*\(")


class ParseError(RuntimeError):
    pass


def strip_comments(text: str) -> str:
    return _COMMENT_RE.sub("", text)


def normalise_lines(text: str) -> List[str]:
    stripped = strip_comments(text)
    lines = []
    for raw in stripped.splitlines():
        line = raw.rstrip()
        if not line:
            lines.append("")
            continue
        stripped_line = line.lstrip()
        if stripped_line.startswith("#include"):
            lines.append(stripped_line)
            continue
        if stripped_line.startswith("#"):
            # Drop other preprocessor guards; reflect-c api will manage guards
            continue
        lines.append(line)
    return lines


def collect_macro(lines: Sequence[str], start: int) -> Tuple[str, int]:
    parts: List[str] = []
    depth = 0
    idx = start
    started = False
    while idx < len(lines):
        token = lines[idx].strip()
        if not token:
            idx += 1
            continue
        parts.append(token)
        opens = token.count("(")
        closes = token.count(")")
        if opens:
            depth += opens
            started = True
        if closes:
            depth -= closes
        idx += 1
        if started and depth <= 0:
            break
    if depth != 0:
        raise ParseError(f"Unbalanced parentheses starting at line {start + 1}")
    macro = " ".join(parts)
    return macro, idx


def parse_line_macro(lines: Sequence[str], start: int) -> Tuple[str, List[str], int]:
    token = lines[start].strip()
    if "(" not in token:
        return token, [], start + 1
    call, next_idx = collect_macro(lines, start)
    name, args = parse_macro(call)
    return name, args, next_idx


def split_args(arg_blob: str) -> List[str]:
    args: List[str] = []
    buf: List[str] = []
    depth = 0
    in_string = False
    escape = False
    for ch in arg_blob:
        if in_string:
            buf.append(ch)
            if escape:
                escape = False
                continue
            if ch == "\\":
                escape = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
            buf.append(ch)
            continue
        if ch == '(':
            depth += 1
            buf.append(ch)
            continue
        if ch == ')':
            depth = max(depth - 1, 0)
            buf.append(ch)
            continue
        if ch == ',' and depth == 0:
            arg = "".join(buf).strip()
            args.append(arg)
            buf = []
            continue
        buf.append(ch)
    final = "".join(buf).strip()
    if final:
        args.append(final)
    return args


def parse_macro(call: str) -> Tuple[str, List[str]]:
    head, _, tail = call.partition("(")
    if not tail:
        raise ParseError(f"Invalid macro call: {call}")
    macro = head.strip()
    body = tail.rsplit(")", 1)[0]
    args = split_args(body)
    return macro, args


def normalise_placeholder(value: str) -> str:
    token = value.strip()
    if token in {"DECOR_BLANK", "INIT_BLANK", "CLEANUP_BLANK", "_", "DECOR"}:
        return "_"
    return token


def normalise_name(value: str) -> str:
    token = value.strip()
    return token


def build_member_from_macro(name: str, args: Sequence[str]) -> Member | None:
    macro = name
    if macro == "FIELD":
        field_name, type_name, _default = args
        return Member("_", "_", normalise_name(type_name), "_", normalise_name(field_name), "_")
    if macro == "FIELD_PTR":
        field_name, type_name, decorator = args
        return Member("_", "_", normalise_name(type_name), normalise_placeholder(decorator), normalise_name(field_name), "_")
    if macro == "FIELD_STRUCT_PTR":
        field_name, type_name, decorator = args
        return Member("_", "struct", normalise_name(type_name), normalise_placeholder(decorator), normalise_name(field_name), "_")
    if macro == "FIELD_ENUM":
        field_name, type_name = args
        return Member("_", "enum", normalise_name(type_name), "_", normalise_name(field_name), "_")
    if macro == "FIELD_PRINTF":
        field_name, type_name, _printf, _scanf = args
        return Member("_", "_", normalise_name(type_name), "_", normalise_name(field_name), "_")
    if macro == "FIELD_CUSTOM":
        field_name, _key, type_name, decorator, *_rest = args
        return Member("_", "_", normalise_name(type_name), normalise_placeholder(decorator), normalise_name(field_name), "_")
    if macro == "FIELD_SNOWFLAKE":
        (field_name,) = args
        return Member("_", "_", "u64snowflake", "_", normalise_name(field_name), "_")
    if macro == "FIELD_BITMASK":
        (field_name,) = args
        return Member("_", "_", "u64bitmask", "_", normalise_name(field_name), "_")
    if macro == "FIELD_TIMESTAMP":
        (field_name,) = args
        return Member("_", "_", "u64unix_ms", "_", normalise_name(field_name), "_")
    if macro in {"COND_WRITE", "COND_END"}:
        return None
    return None


def build_list_members(name: str, args: Sequence[str]) -> List[Member]:
    if name == "LISTTYPE":
        (type_name,) = args
        return [Member("_", "_", normalise_name(type_name), "*", "array", "_")]
    if name == "LISTTYPE_STRUCT":
        (type_name,) = args
        return [Member("_", "struct", normalise_name(type_name), "*", "array", "_")]
    if name == "LISTTYPE_PTR":
        type_name, decorator = args
        return [Member("_", "_", normalise_name(type_name), normalise_placeholder(decorator), "array", "_")]
    return []


def build_enum_entry(name: str, args: Sequence[str]) -> EnumEntry | None:
    if name not in {"ENUMERATOR", "ENUMERATOR_LAST"}:
        return None
    if len(args) != 2:
        raise ParseError(f"Unexpected enum macro signature: {name}({args})")
    enum_name = normalise_name(args[0])
    rhs = args[1].strip()
    if not rhs:
        return EnumEntry(enum_name, "_", "_")
    tokens = rhs.split(None, 1)
    op = tokens[0]
    value = tokens[1] if len(tokens) > 1 else "_"
    return EnumEntry(enum_name, op, value)


def parse_recipe_file(path: pathlib.Path) -> RecipeFile:
    lines = normalise_lines(path.read_text())
    idx = 0
    structs: List[StructDef] = []
    enums: List[EnumDef] = []
    constants: List[ConstantDef] = []
    includes: List[str] = []

    while idx < len(lines):
        line = lines[idx].strip()
        if not line:
            idx += 1
            continue
        if line.startswith("PP_DEFINE"):
            macro, args = parse_macro(line)
            if len(args) != 1:
                raise ParseError(f"Unsupported PP_DEFINE format in {path} -> {line}")
            tokens = args[0].split(None, 1)
            if len(tokens) != 2:
                raise ParseError(f"Expected name/value pair in {line}")
            constants.append(ConstantDef(tokens[0], tokens[1]))
            idx += 1
            continue
        if line.startswith("PP_INCLUDE"):
            macro, args = parse_macro(line)
            if len(args) != 1:
                raise ParseError(f"Unsupported PP_INCLUDE format in {path} -> {line}")
            includes.append(args[0])
            idx += 1
            continue

        if line.startswith("#include"):
            includes.append(line[len("#include"):].strip())
            idx += 1
            continue

        if line.startswith("PUB_STRUCT") or line.startswith("STRUCT"):
            call, next_idx = collect_macro(lines, idx)
            body_lines = []
            while next_idx < len(lines):
                probe = lines[next_idx].strip()
                if probe.startswith("STRUCT_END"):
                    break
                body_lines.append(lines[next_idx])
                next_idx += 1
            idx = next_idx + 1

            macro_name, macro_args = parse_macro(call)
            struct_name = macro_args[0]
            members: List[Member] = []
            inner_idx = 0
            attr_stack: List[str] = []
            while inner_idx < len(body_lines):
                candidate = body_lines[inner_idx].strip()
                if not candidate:
                    inner_idx += 1
                    continue
                if not re.match(r"[A-Za-z_]", candidate):
                    inner_idx += 1
                    continue
                entry_name, entry_args, inner_next = parse_line_macro(body_lines, inner_idx)
                if entry_name == "COND_WRITE":
                    attr_stack.append(OPTIONAL_ATTR)
                    inner_idx = inner_next
                    continue
                if entry_name == "COND_END":
                    if attr_stack:
                        attr_stack.pop()
                    inner_idx = inner_next
                    continue
                member = build_member_from_macro(entry_name, entry_args)
                if member is not None:
                    current_attr = attr_stack[-1] if attr_stack else DEFAULT_ATTR
                    if current_attr != DEFAULT_ATTR:
                        member.attrs = current_attr
                    members.append(member)
                inner_idx = inner_next
            is_public = macro_name == "PUB_STRUCT"
            structs.append(StructDef(struct_name, members, is_public))
            continue

        if line.startswith("PUB_LIST") or line.startswith("LIST"):
            call, next_idx = collect_macro(lines, idx)
            body_lines = []
            while next_idx < len(lines):
                probe = lines[next_idx].strip()
                if probe.startswith("LIST_END"):
                    break
                body_lines.append(lines[next_idx])
                next_idx += 1
            idx = next_idx + 1

            macro_name, macro_args = parse_macro(call)
            struct_name = macro_args[0]
            members = [Member("_", "_", "int", "_", "size", "_")]
            list_idx = 0
            attr_stack: List[str] = []
            while list_idx < len(body_lines):
                snippet = body_lines[list_idx].strip()
                if not snippet:
                    list_idx += 1
                    continue
                entry_name, entry_args, list_next = parse_line_macro(body_lines, list_idx)
                if entry_name == "COND_WRITE":
                    attr_stack.append(OPTIONAL_ATTR)
                    list_idx = list_next
                    continue
                if entry_name == "COND_END":
                    if attr_stack:
                        attr_stack.pop()
                    list_idx = list_next
                    continue
                new_members = build_list_members(entry_name, entry_args)
                for member in new_members:
                    current_attr = attr_stack[-1] if attr_stack else DEFAULT_ATTR
                    if current_attr != DEFAULT_ATTR:
                        member.attrs = current_attr
                    members.append(member)
                list_idx = list_next
            members.append(Member("_", "_", "int", "_", "realsize", "_"))
            is_public = macro_name == "PUB_LIST"
            structs.append(StructDef(struct_name, members, is_public))
            continue

        if line.startswith("ENUM"):
            call, next_idx = collect_macro(lines, idx)
            body_lines = []
            while next_idx < len(lines):
                probe = lines[next_idx].strip()
                if probe.startswith("ENUM_END"):
                    break
                body_lines.append(lines[next_idx])
                next_idx += 1
            idx = next_idx + 1

            _macro_name, macro_args = parse_macro(call)
            enum_name = macro_args[0]
            entries: List[EnumEntry] = []
            inner_idx = 0
            while inner_idx < len(body_lines):
                snippet = body_lines[inner_idx].strip()
                if not snippet:
                    inner_idx += 1
                    continue
                entry_name, entry_args, inner_next = parse_line_macro(body_lines, inner_idx)
                entry = build_enum_entry(entry_name, entry_args)
                if entry is not None:
                    entries.append(entry)
                inner_idx = inner_next
            enums.append(EnumDef(enum_name, entries, True))
            continue

        idx += 1

    return RecipeFile(path, structs, enums, constants, includes)


def ensure_directory(path: pathlib.Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def relative_to(base: pathlib.Path, path: pathlib.Path) -> pathlib.Path:
    try:
        return path.relative_to(base)
    except ValueError as exc:  # pragma: no cover - defensive
        raise ParseError(f"{path} is not inside {base}") from exc


# ---------------------------------------------------------------------------
# Main entry point
# ---------------------------------------------------------------------------


def discover_input_files(root: pathlib.Path) -> List[pathlib.Path]:
    # Skip helper/utility PRE files specific to gencodecs (e.g., gencodecs-process.PRE.h)
    # and the curated aggregator all.PRE.h (kept/maintained under src/api).
    return sorted(
        p
        for p in root.rglob("*.PRE.h")
        if (
            p.is_file()
            and not p.name.startswith("gencodecs")
            and p.name != "all.PRE.h"
            and p.name != "custom.PRE.h"
        )
    )


DEFAULT_OUTPUT = pathlib.Path("src/api")


PRE_INPUT_SUFFIX = ".PRE.h"
RECIPE_OUTPUT_SUFFIX = ".recipe.h"


def transform_to_recipe_path(rel: pathlib.Path) -> pathlib.Path:
    """Return ``rel`` with the legacy .PRE.h suffix swapped for .recipe.h."""
    name = rel.name
    if name.endswith(PRE_INPUT_SUFFIX):
        name = name[: -len(PRE_INPUT_SUFFIX)] + RECIPE_OUTPUT_SUFFIX
    else:
        # Fallback for unexpected suffixes; keep historical behaviour but ensure recipe suffix.
        name = rel.stem + RECIPE_OUTPUT_SUFFIX
    return rel.with_name(name)


def remove_legacy_output(path: pathlib.Path) -> None:
    """Delete leftover .PRE.h artefacts so only *.recipe.h remain."""
    if not path.exists():
        return
    suffixes = path.suffixes
    if suffixes[-2:] == [".PRE", ".h"]:
        path.unlink()


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "input",
        type=pathlib.Path,
        help="Path to gencodecs api root (e.g. gencodecs/api)",
    )
    parser.add_argument(
        "output",
        type=pathlib.Path,
        nargs="?",
        default=DEFAULT_OUTPUT,
        help=f"Directory where reflect-c api will be written (default: {DEFAULT_OUTPUT})",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Parse inputs without writing files",
    )
    args = parser.parse_args(argv)

    inputs = discover_input_files(args.input)
    if not inputs:
        parser.error(f"No .PRE.h files found under {args.input}")

    if not args.dry_run:
        ensure_directory(args.output)

    for infile in inputs:
        try:
            recipe = parse_recipe_file(infile)
        except ParseError as exc:
            print(f"error: failed to parse {infile}: {exc}", file=sys.stderr)
            return 1
        rel = relative_to(args.input, infile)
        rel_recipe = transform_to_recipe_path(rel)
        outfile = args.output / rel_recipe
        if args.dry_run:
            continue
        ensure_directory(outfile.parent)
        remove_legacy_output(args.output / rel)
        outfile.write_text(recipe.render())

    # Always write a reflect-c support recipe with common includes
    if not args.dry_run:
        support_out = args.output / transform_to_recipe_path(pathlib.Path("discord_codecs.PRE.h"))
        ensure_directory(support_out.parent)
        remove_legacy_output(args.output / pathlib.Path("discord_codecs.PRE.h"))
        support_lines = [
            "#ifdef REFLECTC_PRE_HOOKS",
            "/*#!",
            "#include <inttypes.h>",
            "#include <stdbool.h>",
            "#include \"carray.h\"",
            "#include \"cog-utils.h\"",
            "#include \"types.h\"",
            "#include \"concord-error.h\"",
            "#define CONCORD_OPTIONAL (1 << 0ull)",
            "*/",
            "#endif",
            "",
            "PUBLIC(struct, strings, 3, (",
            "        (_, _, int, _, size, _, 0ul),",
            "        (_, _, char, *, array, _, 0ul),",
            "        (_, _, int, _, realsize, _, 0ul)",
            "    )",
            ")",
            "",
            "PUBLIC(struct, json_values, 3, (",
            "        (_, _, int, _, size, _, 0ul),",
            "        (_, _, json_char, *, array, _, 0ul),",
            "        (_, _, int, _, realsize, _, 0ul)",
            "    )",
            ")",
            "",
            "PUBLIC(struct, snowflakes, 3, (",
            "        (_, _, int, _, size, _, 0ul),",
            "        (_, _, u64snowflake, *, array, _, 0ul),",
            "        (_, _, int, _, realsize, _, 0ul)",
            "    )",
            ")",
            "",
            "PUBLIC(struct, bitmasks, 3, (",
            "        (_, _, int, _, size, _, 0ul),",
            "        (_, _, u64bitmask, *, array, _, 0ul),",
            "        (_, _, int, _, realsize, _, 0ul)",
            "    )",
            ")",
            "",
            "PUBLIC(struct, integers, 3, (",
            "        (_, _, int, _, size, _, 0ul),",
            "        (_, _, int, *, array, _, 0ul),",
            "        (_, _, int, _, realsize, _, 0ul)",
            "    )",
            ")",
            "",
            "#ifdef REFLECTC_POST_HOOKS",
            "/*#!",
            "#define reflectc_wrap(_symbol)        reflectc_wrap",
            "",
            "enum {",
            "    REFLECTC_NS_UPPER(_TYPES__u64unix_ms) = REFLECTC_NS_UPPER(_TYPES__EXTEND),",
            "    REFLECTC_NS_UPPER(_TYPES__u64snowflake),",
            "    REFLECTC_NS_UPPER(_TYPES__u64bitmask),",
            "    REFLECTC_NS_UPPER(_TYPES__json_char),",
            "    REFLECTC_NS_UPPER(_TYPES__size_t)",
            "};",
            "*/",
            "#endif",
            "",
        ]
        support_out.write_text("\n".join(support_lines))
    return 0


if __name__ == "__main__":  # pragma: no cover
    raise SystemExit(main())
