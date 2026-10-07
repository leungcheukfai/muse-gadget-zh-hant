#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
# SPDX-License-Identifier: Apache-2.0

"""Host-test streaming Fish Audio WAV conversion."""

import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class MuseTtsWavTest(unittest.TestCase):
    def test_streaming_wav_decode_downmix_and_resample(self):
        compiler = shlex.split(os.environ.get("CC", "cc"))
        if not compiler or shutil.which(compiler[0]) is None:
            self.skipTest("C compiler not available")

        with tempfile.TemporaryDirectory() as tmp:
            binary = Path(tmp) / "muse_tts_wav_test"
            subprocess.run(
                compiler
                + [
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(ROOT / "tests/muse_tts_fakes"),
                    "-I",
                    str(ROOT / "components/muse_zh_hant"),
                    str(ROOT / "tests/muse_tts_wav_harness.c"),
                    str(ROOT / "components/muse_zh_hant/muse_tts_wav.c"),
                    "-o",
                    str(binary),
                ],
                check=True,
            )
            subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    unittest.main()
