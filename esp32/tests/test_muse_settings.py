#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
# SPDX-License-Identifier: Apache-2.0

"""Host-test encrypted Fish Audio key storage and settings migration."""

import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class MuseSettingsTest(unittest.TestCase):
    def test_fish_api_key_storage_contract(self):
        for encryption in (1, 0):
            with self.subTest(encryption=encryption), tempfile.TemporaryDirectory() as tmp:
                binary = Path(tmp) / "muse_settings_test"
                subprocess.run(
                    shlex.split(os.environ.get("CC", "cc"))
                    + [
                        "-std=c11",
                        "-Wall",
                        "-Wextra",
                        "-Werror",
                        f"-DCONFIG_HOMEHUB_NVS_ENCRYPTION={encryption}",
                        "-I",
                        str(ROOT / "tests/muse_settings_fakes"),
                        "-I",
                        str(ROOT / "tests/link_fakes"),
                        "-I",
                        str(ROOT / "components/muse"),
                        "-I",
                        str(ROOT / "components/muse_zh_hant"),
                        str(ROOT / "tests/muse_settings_harness.c"),
                        str(ROOT / "components/muse/muse_settings.c"),
                        "-o",
                        str(binary),
                    ],
                    check=True,
                )
                subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    unittest.main()
