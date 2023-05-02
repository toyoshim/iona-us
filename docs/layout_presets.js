appendPreset('SEGA Basic', [
  // Core : JVS Dash support
  0x00, 0x00, 0x04,
  // Analog Map: Left X-Axis => Lever X, Left Y-Axis => Lever Y
  0x10, 0x11, 0x00, 0x00, 0x00, 0x00, 0x12, 0x13, 0x00, 0x00, 0x00, 0x00,
  // Digital Map
  0x20, 0x00, 0x00, 0x00,  // 1P Up
  0x10, 0x00, 0x00, 0x00,  // 1P Down
  0x08, 0x00, 0x00, 0x00,  // 1P Left
  0x04, 0x00, 0x00, 0x00,  // 1P Right
  0x02, 0x00, 0x00, 0x00,  // 1P B1
  0x01, 0x00, 0x00, 0x00,  // 1P B2
  0x00, 0x80, 0x00, 0x00,  // 1P B3
  0x00, 0x40, 0x00, 0x00,  // 1P B4
  0x00, 0x20, 0x00, 0x00,  // 1P B5
  0x00, 0x10, 0x00, 0x00,  // 1P B6
  0x00, 0x08, 0x00, 0x00,  // 1P B7
  0x00, 0x04, 0x00, 0x00,  // 1P B8
  0x00, 0x00, 0x00, 0x00,  // 1P SHARE
  0x80, 0x00, 0x00, 0x00,  // 1P OPTION
  0x00, 0x00, 0x00, 0x00,  // 1P L3
  0x00, 0x00, 0x00, 0x00,  // 1P R3
  0x00, 0x00, 0x20, 0x00,  // 2P Up
  0x00, 0x00, 0x10, 0x00,  // 2P Down
  0x00, 0x00, 0x08, 0x00,  // 2P Left
  0x00, 0x00, 0x04, 0x00,  // 2P Right
  0x00, 0x00, 0x02, 0x00,  // 2P B1
  0x00, 0x00, 0x01, 0x00,  // 2P B2
  0x00, 0x00, 0x00, 0x80,  // 2P B3
  0x00, 0x00, 0x00, 0x40,  // 2P B4
  0x00, 0x00, 0x00, 0x20,  // 2P B5
  0x00, 0x00, 0x00, 0x10,  // 2P B6
  0x00, 0x00, 0x00, 0x08,  // 2P B7
  0x00, 0x00, 0x00, 0x04,  // 2P B8
  0x00, 0x00, 0x00, 0x00,  // 2P SHARE
  0x00, 0x00, 0x80, 0x00,  // 2P OPTION
  0x00, 0x00, 0x00, 0x00,  // 2P L3
  0x00, 0x00, 0x00, 0x00,  // 2P R3
  // Rapid Fire
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x07,
  0x55, 0x07,
  0x33, 0x07,
  0x03, 0x04,
  0x07, 0x05,
  0xaa, 0x07,
  0xcc, 0x07,
]);

appendPreset('SEGA Virtual-On', [
  // Core : JVS Dash support
  0x00, 0x00, 0x04,
  // Analog Map: Left X-Axis => Lever X, Left Y-Axis => Lever Y
  0x10, 0x11, 0x12, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // Digital Map
  0x20, 0x00, 0x00, 0x00,  // 1P Up
  0x10, 0x00, 0x00, 0x00,  // 1P Down
  0x08, 0x00, 0x00, 0x00,  // 1P Left
  0x04, 0x00, 0x00, 0x00,  // 1P Right
  0x00, 0x80, 0x00, 0x00,  // 1P B1
  0x00, 0x00, 0x00, 0x00,  // 1P B2
  0x00, 0x00, 0x00, 0x00,  // 1P B3
  0x00, 0x00, 0x00, 0x00,  // 1P B4
  0x01, 0x00, 0x00, 0x00,  // 1P B5 L1
  0x00, 0x00, 0x01, 0x00,  // 1P B6 R1
  0x02, 0x00, 0x00, 0x00,  // 1P B7 L2
  0x00, 0x00, 0x02, 0x00,  // 1P B8 R2
  0x00, 0x00, 0x00, 0x00,  // 1P SHARE
  0x80, 0x00, 0x00, 0x00,  // 1P OPTION
  0x00, 0x00, 0x00, 0x00,  // 1P L3
  0x00, 0x00, 0x00, 0x00,  // 1P R3
  0x00, 0x00, 0x20, 0x00,  // 2P Up
  0x00, 0x00, 0x10, 0x00,  // 2P Down
  0x00, 0x00, 0x08, 0x00,  // 2P Left
  0x00, 0x00, 0x04, 0x00,  // 2P Right
  0x00, 0x00, 0x00, 0x00,  // 2P B1
  0x00, 0x00, 0x00, 0x00,  // 2P B2
  0x00, 0x00, 0x00, 0x00,  // 2P B3
  0x00, 0x00, 0x00, 0x00,  // 2P B4
  0x00, 0x00, 0x00, 0x00,  // 2P B5
  0x00, 0x00, 0x00, 0x00,  // 2P B6
  0x00, 0x00, 0x00, 0x00,  // 2P B7
  0x00, 0x00, 0x00, 0x00,  // 2P B8
  0x00, 0x00, 0x00, 0x00,  // 2P SHARE
  0x00, 0x00, 0x00, 0x00,  // 2P OPTION
  0x00, 0x00, 0x00, 0x00,  // 2P L3
  0x00, 0x00, 0x00, 0x00,  // 2P R3
  // Rapid Fire
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x07,
  0x55, 0x07,
  0x33, 0x07,
  0x03, 0x04,
  0x07, 0x05,
  0xaa, 0x07,
  0xcc, 0x07,
]);