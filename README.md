# Video Downloader
A video downloader with Qt GUI.

Currently YouTube is the only maintained platform so this is basically a YouTube downloader.
Is also allows to download any quality, including the HD qualities. However
1080p streams (and above) are only provided as video-only or audio-only stream
by YouTube which currently need to be downloaded separately and then muxed together.

This is just a downloader. It does not convert or mux anything. You might use
ffmpeg to convert/remux downloaded videos.

TODO:
 * Remove platforms that are not working.
 * Mux video-only and audio-only streams.
 * Command line interface

## Build instructions
The video downloader depends on c++utilities and qtutilities and is built in the same way as these libaries.

The following Qt 5 modules are requried: core gui widgets network
