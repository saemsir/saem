#!/usr/bin/env python3
"""Build a safe, static snapshot of the latest public Saem Physics uploads."""

import datetime
import json
import re
import sys
import urllib.request
import xml.etree.ElementTree as ET
from pathlib import Path

CHANNEL_ID = "UCJlNb8kQgLWqb65htic2CkQ"
FEED_URL = f"https://www.youtube.com/feeds/videos.xml?channel_id={CHANNEL_ID}"
ATOM = "{http://www.w3.org/2005/Atom}"
YOUTUBE = "{http://www.youtube.com/xml/schemas/2015}"
MEDIA = "{http://search.yahoo.com/mrss/}"
VIDEO_ID = re.compile(r"[A-Za-z0-9_-]{11}\Z")
MAX_ITEMS = 15
MAX_FEED_BYTES = 1_000_000


def parse_feed(xml_data):
    root = ET.fromstring(xml_data)
    channel_id = root.findtext(f"{YOUTUBE}channelId")
    if not channel_id or channel_id.removeprefix("UC") != CHANNEL_ID.removeprefix("UC"):
        raise ValueError(f"Unexpected YouTube channel ID: {channel_id!r}")

    items = []
    for entry in root.findall(f"{ATOM}entry"):
        video_id = entry.findtext(f"{YOUTUBE}videoId", default="")
        title = entry.findtext(f"{ATOM}title", default="").strip()
        published = entry.findtext(f"{ATOM}published", default="")
        group = entry.find(f"{MEDIA}group")
        description = group.findtext(f"{MEDIA}description", default="").strip()[:1000] if group is not None else ""
        if not VIDEO_ID.fullmatch(video_id) or not title or not published:
            continue
        items.append({
            "videoId": video_id,
            "title": title,
            "published": published,
            "description": description,
        })
        if len(items) == MAX_ITEMS:
            break
    if not items:
        raise ValueError("The YouTube feed contained no valid videos")
    return items


def fetch_feed():
    request = urllib.request.Request(
        FEED_URL,
        headers={"User-Agent": "SaemPhysicsSite/1.0 (+https://saemsir.github.io/saem/)"},
    )
    with urllib.request.urlopen(request, timeout=30) as response:
        if response.status != 200:
            raise RuntimeError(f"YouTube feed returned HTTP {response.status}")
        xml_data = response.read(MAX_FEED_BYTES + 1)
        if len(xml_data) > MAX_FEED_BYTES:
            raise ValueError(f"YouTube feed exceeds the {MAX_FEED_BYTES}-byte limit")
        return xml_data


def main(argv):
    if len(argv) != 2:
        print(f"Usage: {Path(argv[0]).name} OUTPUT.json", file=sys.stderr)
        return 2
    try:
        items = parse_feed(fetch_feed())
        payload = {
            "channelId": CHANNEL_ID,
            "generatedAt": datetime.datetime.now(datetime.timezone.utc)
            .isoformat(timespec="seconds")
            .replace("+00:00", "Z"),
            "items": items,
        }
        Path(argv[1]).write_text(
            json.dumps(payload, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
    except (OSError, RuntimeError, ValueError, ET.ParseError) as error:
        print(f"Could not refresh the Saem Physics YouTube feed: {error}", file=sys.stderr)
        return 1
    print(f"Wrote {len(items)} recent uploads from channel {CHANNEL_ID}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
