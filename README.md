# Lattice

![Screenshot](/../screenshots/screenshot.png?raw=true "Lattice Screenshot")

Lattice is a lightweight status hub for linux that displays info such as the current date, time, and audio settings. Its goal was to replace i3bar in my current setup so that I could remove the ugly, wasted space at the bottom of my screen without scarificing convenience.

There is no configuration file yet, so any visual (or functional) changes will require a recompile. Also, the audio module gets its info from a custom script that I use to control my audio, but it should be very straightforward to swap the implementation to use something else.

Lattice makes use of transparency, so you will need a [composite manager](https://wiki.archlinux.org/index.php?title=Xorg&redirect=no#Composite).

## Compiling

Dependencies:
+ X11
+ Xinerama
+ Cairo/Pango

To compile, `make` should suffice. There is no install target, so you'll need to move `lattice` and `lattice-msg` into your path.

## Usage

To start Lattice, add `lattice &` to your startup script.

Once running, you can interact with the hub using `lattice-msg`:
```bash
lattice-msg hide
lattice-msg show
lattice-msg show-update (audio|date|time|workspace)
lattice-msg stop
lattice-msg toggle
```

The `toggle` command will toggle between showing and hiding, `stop` will tell Lattice to shutdown, and `show-update` will show a single module for a couple seconds before fading away.

I recommend using these in your window manager configuration. I have `lattice-msg toggle` bound to `Super + Space`, and `lattice-msg show-update audio` bound to all of my media keys.
