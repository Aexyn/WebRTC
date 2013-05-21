/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SRC_VOICE_ENGINE_MAIN_TEST_AUTO_TEST_STANDARD_TEST_BASE_AFTER_INIT_H_
#define SRC_VOICE_ENGINE_MAIN_TEST_AUTO_TEST_STANDARD_TEST_BASE_AFTER_INIT_H_

#include "webrtc/common_types.h"
#include "webrtc/system_wrappers/interface/scoped_ptr.h"
#include "webrtc/voice_engine/test/auto_test/fixtures/before_initialization_fixture.h"

class TestErrorObserver;

class LoopBackTransport : public webrtc::Transport {
 public:
  LoopBackTransport(webrtc::VoENetwork* voe_network)
      : voe_network_(voe_network) {
  }

  virtual int SendPacket(int channel, const void *data, int len) {
    voe_network_->ReceivedRTPPacket(channel, data, len);
    return len;
  }

  virtual int SendRTCPPacket(int channel, const void *data, int len) {
    voe_network_->ReceivedRTCPPacket(channel, data, len);
    return len;
  }

 private:
  webrtc::VoENetwork* voe_network_;
};

// This fixture initializes the voice engine in addition to the work
// done by the before-initialization fixture. It also registers an error
// observer which will fail tests on error callbacks. This fixture is
// useful to tests that want to run before we have started any form of
// streaming through the voice engine.
class AfterInitializationFixture : public BeforeInitializationFixture {
 public:
  AfterInitializationFixture();
  virtual ~AfterInitializationFixture();
 protected:
  webrtc::scoped_ptr<TestErrorObserver> error_observer_;
};

#endif  // SRC_VOICE_ENGINE_MAIN_TEST_AUTO_TEST_STANDARD_TEST_BASE_AFTER_INIT_H_
