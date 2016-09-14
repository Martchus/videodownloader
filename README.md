# Video Downloader
A video downloader with Qt 5 GUI.

Currently YouTube and Vimeo are the only maintained platforms.

This is just a downloader. It does not convert or mux anything. You might use
ffmpeg or mkvmerge to convert/remux downloaded videos.

It seems that not all YouTube videos work anymore. Since this is mainly a
learning/test project for me I currently have no intention to fix it.

## Supported YouTube quality levels
The downloader allows to download any quality, including the HD qualities. However
1080p streams (and above) are only provided as video-only or audio-only stream
by YouTube which currently need to be downloaded separately and then muxed together.

## TODO
 * Fix (or remove) platforms that are not working.
 * Mux video-only and audio-only streams.
 * Command line interface
 * Fix issues with some YouTube videos

## Download / binary repository
I currently provide packages for Arch Linux and Windows. Sources for those packages can be found in a
separate [repository](https://github.com/Martchus/PKGBUILDs). For binaries checkout my
[website](http://martchus.no-ip.biz/website/page.php?name=programming).

## Build instructions
The video downloader depends on c++utilities and qtutilities and is built in the same way as these libaries.

The following Qt 5 modules are requried: core gui widgets network
