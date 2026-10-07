## Start the Raspberry Pi desktop setup automatically

The launcher in `scripts/start-desktop.sh` opens scripts 0 through 5 in six
separate LXTerminal windows, starting them in numeric order one second apart.
The scripts continue running at the same time. It expects this checkout at
`/home/gzalo/hugo-re`, as the component scripts currently use that path.

On the Pi, make sure `lxterminal` is installed, then register the launcher for
the desktop user's session:

```sh
command -v lxterminal || sudo apt install lxterminal
mkdir -p ~/.config/autostart
cp /home/gzalo/hugo-re/scripts/hugo-autostart.desktop ~/.config/autostart/
```

Log into the desktop again (or reboot with desktop autologin enabled) to start
the windows. To try it immediately from a desktop terminal, run:

```sh
/bin/sh /home/gzalo/hugo-re/scripts/start-desktop.sh
```

If the Pi uses a different username or checkout location, update the `Exec=`
path in the `.desktop` file and the absolute paths inside the component scripts.
The phone scripts also require the `pjsua2_hello_reg.exe` binary to be built in
`tools/pjsua-to-keys/`; the binary is not included in this repository. The
audio server script's `sudo chmod` may ask for a password during startup.
