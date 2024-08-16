# remindme

Lightweight system reminders for the command line.

## Installation

### Arch Linux

If you use Arch Linux, you can install [`remind-me-git`](https://aur.archlinux.org/packages/remind-me-git) from the Arch User Repository (AUR) using an AUR helper like `yay`:

```bash
yay -S remind-me-git
```

Then, enable and start the `remindd` service:

```bash
systemctl --user daemon-reload
systemctl --user enable remindd.service
systemctl --user start remindd.service
```

### Manual from Source

```bash
git clone https://github.com/carrotfarmer/remindme.git
cd remindme
make
sudo cp remindme /usr/local/bin # CLI client
sudo cp remindd /usr/local/bin # reminder daemon
sudo cp remindd.service /usr/lib/systemd/system/ # move the daemon service file to the systemd directory
```

Then, enable and start the `remindd` service:

```bash
systemctl --user enable remindd.service
systemctl --user start remindd.service
```

## Usage

First, ensure that the `remindd` service is running:

```bash
systemctl --user status remindd.service
```

To add a reminder, use the `remindme` command with the reminder message and time:

<!-- explain the date formats -->

There are two ways you can specify the time for the reminder:

1. You can specify the time in the [ISO-8601 date-format](https://en.wikipedia.org/wiki/ISO_8601) combined with the 24-hour time: `YYYY-MM-DD HH:MM`
2. You can specify relative times using `d`, `h`, `m`, `s` for days, hours, minutes, and seconds. Example: `1d 2h 3m`

```bash
# sets reminder for August 31, 2024 at 11:59 PM
remindme "Take out the trash" 2024/08/31 23:59

# sets reminder for 1 day, 2 hours, and 3 minutes from the current time
remindme "Take out the trash" 1d 2h 3m

# sets reminder for 35 minutes and 30 seconds from the current time
remindme "Take out the trash" 35m 30s
```

To list all reminders, use the `remindme` command without any arguments:

```bash
remindme
```

To remove a reminder, use the `remindme` command with the `-d` flag and the reminder ID:

```bash
remindme -d 3 
```

You can also clear all reminders:

```bash
remindme --clear-all
```
