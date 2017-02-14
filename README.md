# Minfo

![Screenshot](/../screenshots/screenshot.png?raw=true "Minfo Screenshot")

Minfo is a lightweight status hub for displaying info such as the current date, time, and audio settings. Its goal is to replace i3bar in my current setup so that I can remove the ugly, wasted space at the bottom of my screen without scarificing the convenience.

There is no configuration file yet, so any visual (or functional) changes will require a recompile. Also, the audio module gets its info from a custom script that I use to control my audio, but it should be very straightforward to swap the implementation to use something else.

## Compiling

Dependencies:
+ X11
+ Xinerama
+ Cairo/Pango

To compile, `make` should suffice. There is no install target, so you'll need to move `minfo` and `minfo-msg` into your path.

## Usage

To start minfo, add `minfo &` to your startup script.

Once running, you can interact with the hub using `minfo-msg`:
```bash
minfo-msg hide
minfo-msg show
minfo-msg show-update (audio|date|time)
minfo-msg stop
minfo-msg toggle
```

The `toggle` command will toggle between showing and hiding, `stop` will tell minfo to shutdown, and `show-update` will show a single module for a couple seconds before fading away.

I recommend using these in your window manager configuration. I have `minfo-msg toggle` bound to `Super + Space`, and `minfo-msg show-update audio` bound to all of my media keys.
