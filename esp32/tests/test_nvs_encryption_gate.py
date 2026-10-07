#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
# SPDX-License-Identifier: Apache-2.0

"""Verify encrypted firmware requires an existing eFuse HMAC key."""

import os
from pathlib import Path
import shlex
import shutil
import signal
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class NvsEncryptionGateTest(unittest.TestCase):
    def test_preprovisioned_key_required(self):
        compiler = shlex.split(os.environ.get("CC", "cc"))
        if not compiler or shutil.which(compiler[0]) is None:
            self.skipTest("C compiler not available")

        with tempfile.TemporaryDirectory() as tmp:
            binary = Path(tmp) / "nvs_encryption_gate_test"
            subprocess.run(
                compiler
                + [
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-DCONFIG_NVS_ENCRYPTION=1",
                    "-DCONFIG_HOMEHUB_NVS_REQUIRE_EXISTING_HMAC=1",
                    "-DCONFIG_NVS_SEC_HMAC_EFUSE_KEY_ID=0",
                    "-I",
                    str(ROOT / "tests/nvs_encryption_fakes"),
                    "-I",
                    str(ROOT / "tests/link_fakes"),
                    "-I",
                    str(ROOT / "main"),
                    str(ROOT / "tests/nvs_encryption_harness.c"),
                    str(ROOT / "main/config_store.c"),
                    "-o",
                    str(binary),
                ],
                check=True,
            )
            subprocess.run([str(binary), "provisioned"], check=True)
            result = subprocess.run([str(binary), "unprovisioned"], check=False)
            self.assertEqual(result.returncode, -signal.SIGABRT)


if __name__ == "__main__":
    unittest.main()
