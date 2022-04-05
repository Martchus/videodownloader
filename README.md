# Video Downloader
A video downloader with Qt GUI and backends for multiple platforms, e.g. YouTube and
Vimeo.

---

**Note**: This project is not maintained anymore so any provider-specific code might be
outdated and not work anymore. I keep the project around as an example for doing HTTP
downloads with Qt showing the progress and speed in a list view.

---

This is just a downloader. It does not convert or mux anything. You might use
ffmpeg or mkvmerge to convert/remux downloaded videos.

It seems that not all YouTube videos work anymore. Since this is mainly a
learning/test project for me I currently have no intention to fix it.

## Supported YouTube quality levels
The downloader allows to download any quality, including the HD qualities. However
1080p streams (and above) are only provided as video-only or audio-only stream
by YouTube which currently need to be downloaded separately and then muxed together.

## Build instructions
The video downloader depends on c++utilities and qtutilities and is built in the same
way as these libraries.

The following Qt modules are required: core gui widgets network

## Copyright notice and license
Copyright © 2015-2022 Marius Kittler

All code is licensed under [GPL-2-or-later](LICENSE).
