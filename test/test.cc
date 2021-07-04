// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include <stdint.h>

extern "C" {
#include "chlib/usb.h"
#include "hid.h"
}

#include "gtest/gtest.h"
#include "mock.h"

namespace anonymous {

struct DummyDescriptor {
  usb_desc_configuration configuration = {
    sizeof(usb_desc_configuration),
    USB_DESC_CONFIGURATION,
    sizeof(DummyDescriptor),
  };
  usb_desc_endpoint endpoint {
    sizeof(usb_desc_endpoint),
    USB_DESC_ENDPOINT,
    129,
  };
  usb_desc_hid hid = {
    sizeof(usb_desc_hid),
    USB_DESC_HID,
    0x0101,
    0x00,
    0x01,
    USB_DESC_HID_REPORT,
    0x0000,
  };
} usb_desc;

class PS4CompatTest : public ::testing::Test {
 protected:
  void SetReportSize(uint16_t report_size) {
    ASSERT_TRUE(usb_host);
    ASSERT_TRUE(usb_host->check_configuration_desc);

    usb_desc.hid.wDescriptorLength = report_size;
    usb_host->check_configuration_desc(
        0, reinterpret_cast<const uint8_t*>(&usb_desc));
  }

  void CheckHidReportDescriptor(const uint8_t* desc) {
    ASSERT_TRUE(usb_host->check_hid_report_desc);

    usb_host->check_hid_report_desc(0, desc);
  }

  void CheckHubInfo(hub_info& expected, hub_info& actual) {
    EXPECT_EQ(expected.report_desc_size, actual.report_desc_size);
    EXPECT_EQ(expected.report_size, actual.report_size);
    for (size_t i = 0; i < 2; ++i) {
      EXPECT_EQ(expected.axis[i], actual.axis[i]);
      EXPECT_EQ(expected.axis_size[i], actual.axis_size[i]);
    }
    for (size_t i = 0; i < 12; ++i)
      EXPECT_EQ(expected.button[i], actual.button[i]);
    EXPECT_EQ(expected.dpad, actual.dpad);
    EXPECT_EQ(expected.report_id, actual.report_id);
    EXPECT_EQ(expected.type, actual.type);
    EXPECT_EQ(expected.ep, actual.ep);
    EXPECT_EQ(expected.state, actual.state);
  }

 private:
  void SetUp() override {
    hid.report = nullptr;
    hid_init(&hid);
  }

  hid hid;
};

TEST_F(PS4CompatTest, HoripadFpsPlusForPlayStation4) {
  const uint8_t hid_report_desc[] = {
    0x05, 0x01, 0x09, 0x05, 0xa1, 0x01, 0x85, 0x01,
    0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x35,
    0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08, 0x95,
    0x04, 0x81, 0x02, 0x09, 0x39, 0x15, 0x00, 0x25,
    0x07, 0x35, 0x00, 0x46, 0x3b, 0x01, 0x65, 0x14,
    0x75, 0x04, 0x95, 0x01, 0x81, 0x42, 0x65, 0x00,
    0x05, 0x09, 0x19, 0x01, 0x29, 0x0e, 0x15, 0x00,
    0x25, 0x01, 0x75, 0x01, 0x95, 0x0e, 0x81, 0x02,
    0x06, 0x00, 0xff, 0x09, 0x20, 0x75, 0x06, 0x95,
    0x01, 0x81, 0x02, 0x05, 0x01, 0x09, 0x33, 0x09,
    0x34, 0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08,
    0x95, 0x02, 0x81, 0x02, 0x06, 0x00, 0xff, 0x09,
    0x21, 0x95, 0x36, 0x81, 0x02, 0x85, 0x05, 0x09,
    0x22, 0x95, 0x1f, 0x91, 0x02, 0x85, 0x03, 0x0a,
    0x21, 0x27, 0x95, 0x2f, 0xb1, 0x02, 0xc0, 0x06,
    0xf0, 0xff, 0x09, 0x40, 0xa1, 0x01, 0x85, 0xf0,
    0x09, 0x47, 0x95, 0x3f, 0xb1, 0x02, 0x85, 0xf1,
    0x09, 0x48, 0x95, 0x3f, 0xb1, 0x02, 0x85, 0xf2,
    0x09, 0x49, 0x95, 0x0f, 0xb1, 0x02, 0x85, 0xf3,
    0x0a, 0x01, 0x47, 0x95, 0x07, 0xb1, 0x02, 0xc0,
  };
  hub_info expected = {
    sizeof(hid_report_desc),
    504,
    { 0, 8, },
    32,
    { 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 },
    { 8, 8 },
    1,
    HID_TYPE_PS4,
    129,
    HID_STATE_READY,
  };
  SetReportSize(sizeof(hid_report_desc));
  CheckHidReportDescriptor(hid_report_desc);
  CheckHubInfo(expected, *hid_get_info(0));
}

}