"""Use the local firmware password for PlatformIO OTA uploads."""

import ast
import os
from pathlib import Path
import re
import subprocess

Import("env")


def read_password(config_path):
    override = os.environ.get("OTA_PASSWORD")
    if override:
        return override

    config = Path(config_path).read_text(encoding="utf-8-sig")
    match = re.search(
        r'^[ \t]*#[ \t]*define[ \t]+OTA_PASSWORD[ \t]+'
        r'("(?:\\.|[^"\\\r\n])*")[ \t]*(?://[^\r\n]*)?$',
        config,
        re.MULTILINE,
    )
    if not match:
        raise ValueError("Set OTA_PASSWORD to a quoted string in include/config.h")

    try:
        password = ast.literal_eval(match.group(1))
    except (SyntaxError, ValueError):
        raise ValueError("OTA_PASSWORD in include/config.h is not a valid string") from None

    if not password or "\0" in password:
        raise ValueError("Set a nonempty OTA_PASSWORD in include/config.h")
    return password


def upload_firmware(source, target, env):
    project_dir = Path(env.subst("$PROJECT_DIR"))
    try:
        password = read_password(project_dir / "include" / "config.h")
    except (OSError, ValueError) as error:
        print("OTA upload stopped: " + str(error))
        return 1

    # Keep platform options such as the destination, ports and filesystem flag.
    # Suppress espota's debug option because it prints the authentication password.
    flags = []
    skip_next = False
    for flag in env.subst_list("$UPLOADERFLAGS", source=source, target=target)[0]:
        flag = str(flag)
        if skip_next:
            skip_next = False
            continue
        if flag in ("--auth", "-a"):
            skip_next = True
        elif flag not in ("--debug", "-d") and not flag.startswith("--auth="):
            flags.append(flag)

    command = [env.subst("$PYTHONEXE"), env.subst("$UPLOADER")]
    command.extend(flags)
    command.extend(["--auth=" + password, "--file", str(source[0])])
    print("Using the configured OTA password (hidden)")
    return subprocess.call(command, cwd=project_dir)


env.Replace(UPLOADCMD=upload_firmware)
