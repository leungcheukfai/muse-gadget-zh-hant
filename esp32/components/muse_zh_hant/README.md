# Traditional Chinese extension component

`muse_zh_hant` provides the Traditional Chinese language table, CJK font
fallback, and Fish Audio speech client. The Muse UI, BLE setup page, and chat
session call the component through its public headers. Fish Audio handles both
Cantonese and Mandarin; users supply their own API key and voice model IDs.

The Muse SDK does not load firmware components at runtime. This component is
linked into the application image, so enabling it requires rebuilding and
flashing the board-specific firmware. It is a source add-on, not a drop-in
binary plugin.
