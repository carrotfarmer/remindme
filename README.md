# remindme

Lightweight system reminders for the command line.

## Installation

### Arch Linux

If you use Arch Linux, you can install [`remindme-cli`](https://aur.archlinux.org/packages/remindme-cli) from the Arch User Repository (AUR) using an AUR helper like `yay`:

```bash
yay -S remindme-cli
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
sudo cp systemd/remindd.service /usr/lib/systemd/system/ # move the daemon service file to the systemd directory
```

Then, enable and start the `remindd` service:

```bash
sudo systemctl --user enable remindd.service
sudo systemctl --user start remindd.service
```

## Usage

First, ensure that the `remindd` service is running:

```bash
systemctl --user status remindd.service
```

To add a reminder, use the `remindme` command:

```bash
remindme "Take out the trash" 08/09/2024 15:00
```

Dates must be in the format `MM/DD/YYYY` and times must be in the 24-hour format `HH:MM`.

To list all reminders, use the `remindme` command without any arguments:

```bash
remindme
```

To remove a reminder, use the `remindme` command with the `-d` flag and the reminder ID:

```bash
remindme -d 11025
```

You can also clear all reminders:

```bash
remindme --clear-all
```
