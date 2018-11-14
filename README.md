chbl
====
Change the brightness of your screen backlight.

The target brightness can be specified using relative values
```shell
chbl intel_backlight 50%     # set brightness to 50%
chbl intel_backlight +5%     # increase brightness by 5%
chbl intel_backlight -5%     # decrease brightness by 5%
```
and absolute values
```shell
chbl intel_backlight 200     # set brightness to 200
chbl intel_backlight +10     # increase brightness by 10
chbl intel_backlight -10     # decrease brightness by 10
```
Replace `intel_backlight` by the name of your backlight (can be found via `ls -1 /sys/class/backlight`).

A udev rule can be used to allow a normal user to change the brightness. Just add the corresponding users to the `video` group and create the file `/etc/udev/rules.d/backlight.rules` with the contents (see [Archlinux Wiki](https://wiki.archlinux.org/index.php/backlight))
```
ACTION=="add", SUBSYSTEM=="backlight", KERNEL=="acpi_video0", RUN+="/bin/chgrp video /sys/class/backlight/%k/brightness"
ACTION=="add", SUBSYSTEM=="backlight", KERNEL=="acpi_video0", RUN+="/bin/chmod g+w /sys/class/backlight/%k/brightness"
```
