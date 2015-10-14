# Video Downloader
A video downloader with Qt 5 GUI.

Currently YouTube and Vimeo are the only maintained platforms.

This is just a downloader. It does not convert or mux anything. You might use
ffmpeg or mkvmerge to convert/remux downloaded videos.

## Supported YouTube quality levels

The downloader allows to download any quality, including the HD qualities. However
1080p streams (and above) are only provided as video-only or audio-only stream
by YouTube which currently need to be downloaded separately and then muxed together.

## TODO
 * Fix (or remove) platforms that are not working.
 * Mux video-only and audio-only streams.
 * Command line interface

## Build instructions
The video downloader depends on c++utilities and qtutilities and is built in the same way as these libaries.

The following Qt 5 modules are requried: core gui widgets network
