import unittest

from fetch_channel_feed import CHANNEL_ID, MAX_ITEMS, parse_feed


def make_feed(entries, channel_id=CHANNEL_ID):
    return f"""<?xml version="1.0"?>
    <feed xmlns="http://www.w3.org/2005/Atom"
          xmlns:yt="http://www.youtube.com/xml/schemas/2015"
          xmlns:media="http://search.yahoo.com/mrss/">
      <yt:channelId>{channel_id}</yt:channelId>
      {entries}
    </feed>"""


def make_entry(video_id, title="Physics &amp; intuition", published="2026-11-08T10:00:00+00:00"):
    return f"""<entry>
      <yt:videoId>{video_id}</yt:videoId>
      <title>{title}</title>
      <published>{published}</published>
      <media:group><media:description>Explore the physics idea.</media:description></media:group>
    </entry>"""


class ChannelFeedTests(unittest.TestCase):
    def test_reads_recent_video_metadata(self):
        xml_data = make_feed(make_entry("AbCdEf12345"))
        self.assertEqual(
            parse_feed(xml_data),
            [{
                "videoId": "AbCdEf12345",
                "title": "Physics & intuition",
                "published": "2026-11-08T10:00:00+00:00",
                "description": "Explore the physics idea.",
            }],
        )

    def test_bounds_and_trims_upload_descriptions(self):
        long_text = " " + ("x" * 1100) + " "
        xml_data = make_feed(make_entry("AbCdEf12345").replace(
            "Explore the physics idea.", long_text
        ))
        description = parse_feed(xml_data)[0]["description"]
        self.assertEqual(len(description), 1000)
        self.assertEqual(description[0], "x")

    def test_rejects_a_different_channel(self):
        with self.assertRaisesRegex(ValueError, "Unexpected YouTube channel"):
            parse_feed(make_feed(make_entry("AbCdEf12345"), "UCnotSaem"))

    def test_accepts_youtubes_short_channel_id_form(self):
        xml_data = make_feed(make_entry("AbCdEf12345"), CHANNEL_ID.removeprefix("UC"))
        self.assertEqual(len(parse_feed(xml_data)), 1)

    def test_ignores_malformed_entries_and_requires_a_valid_video(self):
        xml_data = make_feed(make_entry("not-a-video-id"))
        with self.assertRaisesRegex(ValueError, "no valid videos"):
            parse_feed(xml_data)

    def test_caps_the_feed_snapshot(self):
        entries = "".join(
            make_entry(f"{index:011d}", title=f"Video {index}")
            for index in range(MAX_ITEMS + 3)
        )
        self.assertEqual(len(parse_feed(make_feed(entries))), MAX_ITEMS)


if __name__ == "__main__":
    unittest.main()
