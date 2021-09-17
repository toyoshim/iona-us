---
layout: default_en
title: Troubleshooting
permalink: /troubleshoot_en
---
# Troubleshooting (under translation)
---
## Playbook to shoot troubles
1. LED does not blink on power-on
   1. Do you supply 5V power via micro USB port correctly?
      - Supply 5V power.
   2. Don't you press SERVICE button on boot.
      - Do not press SERVICE button on booting.
   3. Did you fail firmware update?
      - You can retry firmware updates even if the written firmware is broken.
      Follow the steps described in the [Firmware Update](firmware_en) page.
   4. Other cases.
      - The device may get broken during or after shipping.
      Feel free to contact the author.

2. JVS based systems don't recognize IONA-US.
   1. Do you connect IONA's JVS port to the JVS system correctly?
      - Connect them for JVS bus with a general USB cable.
   2. Are jumpper pins for firmware updates shorten?
      - As it makes JVS but not to work, please open these pins.
   3. Are LED blinking or off?
      - It should blink after the initial boot, blink fastly on JVS but reset, turn on when an address is assigned over JVS. Unless an address is assigned, it should not be recognized.
      As this may be a compatibility issue, feel free to contact author with LED status.
   4. Other cases.
      - May be a compatibility issue for IONA-US and the arcade system board.
      It may be resolved by a firmware update if you report the issue.
      If the title is not listed at 
      [Compatibility Information](https://github.com/toyoshim/iona/wiki/Compatibility-Information),
      it's just unsupported. If it is, there may be an electrical compatibility issue.
      Etherway, firmware update may be able to solve the issue if you report.

3. USB gamepads connected with IONA-US don't work
   1. Do you use USB hub?
      - Do not use a hub.
   2. Do you enter any setting mode other than the normal mode.
      - If you enter any configuration mode by pressing TEST+SERVICE,
      inputs from gamepad are not sent to the system board over JVS.
      Please quit the setting mode.
   3. Do you see LED flash on connecting the USB gamepad?
      - It doesn't flash if the gamepad is not recognized in confidence.
      It wouldn't be an officially supported device.
      Feel free to request the device support at the [Report](report_en) page with informatino for the device.
   4. Do you configure custom layout correctly?
      - It's possible that wrong configuration omit inputs.
      Please follow the [manual](en) to configure custom layout as you want, or call factory settings.
      If the problem persists, feel free to report the problem at the [Report](report_en) page with information for the device.
   5. Do you enable the twinstick mode for the 1P gamepad?
      - If the twinstick mode is enabled for the 1P gamepad, 2P gamepad will be ignored.
      Please use it in the normal mode.
   6. Other cases.
      - Feel free to report the issue at the [Report](report_en) page with information for the device.

4. Unstable behaviors
   1. Power supply from the micro USB port is enough?
      - Try another PC, a USB hub with power supply, a more powerful charger
   2. Do you enable the twinstick mode?
      - A special layout is enforced in the twinstick mode.
      Please check the [Manual](en), and disable the twinstick mode for usual uses.
   3. Other cases.
      - Feel free to contact the author.

## Contact
If the playbook suggests report issues, follow the steps described in [Report](report_en) page.
Otherwise, feel free to contact [the author at Twitter](https://twitter.com/toyoshim).
You can use the Twitter's mention feature, or send a DM as it's open for everyone.
As long as notification is sent to me, I will reply.