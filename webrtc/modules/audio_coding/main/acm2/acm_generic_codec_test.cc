/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "testing/gtest/include/gtest/gtest.h"
#include "webrtc/base/safe_conversions.h"
#include "webrtc/modules/audio_coding/main/acm2/acm_generic_codec.h"

namespace webrtc {
namespace acm2 {

namespace {
const int kDataLengthSamples = 80;
const int16_t kZeroData[kDataLengthSamples] = {0};
const CodecInst kDefaultCodecInst =
    {0, "pcmu", 8000, 2 * kDataLengthSamples, 1, 64000};
const int kCngPt = 13;
const int kNoCngPt = 255;
const int kRedPt = 255;  // Not using RED in this test.
}  // namespace

class AcmGenericCodecTest : public ::testing::Test {
 protected:
  AcmGenericCodecTest() : timestamp_(0) {
    acm_codec_params_ = {kDefaultCodecInst, true, true, VADNormal};
  }

  void CreateCodec() {
    codec_.reset(new ACMGenericCodec(acm_codec_params_.codec_inst, kCngPt,
                                     kNoCngPt, kNoCngPt, kNoCngPt,
                                     false /* enable RED */, kRedPt));
    ASSERT_TRUE(codec_);
    ASSERT_EQ(0, codec_->InitEncoder(&acm_codec_params_, true));
  }

  void EncodeAndVerify(int expected_return_val,
                       size_t expected_out_length,
                       uint32_t expected_timestamp,
                       WebRtcACMEncodingType expected_encoding_type,
                       int expected_payload_type,
                       int expected_send_even_if_empty) {
    uint8_t out[kDataLengthSamples];
    int16_t out_length;
    WebRtcACMEncodingType encoding_type;
    AudioEncoder::EncodedInfo encoded_info;
    EXPECT_EQ(expected_return_val,
              codec_->Encode(timestamp_, kZeroData, kDataLengthSamples, 1, out,
                             &out_length,&encoding_type, &encoded_info));
    timestamp_ += kDataLengthSamples;
    EXPECT_EQ(expected_encoding_type, encoding_type);
    EXPECT_TRUE(encoded_info.redundant.empty());
    EXPECT_EQ(expected_out_length, encoded_info.encoded_bytes);
    EXPECT_EQ(expected_out_length, rtc::checked_cast<size_t>(out_length));
    EXPECT_EQ(expected_timestamp, encoded_info.encoded_timestamp);
    if (expected_payload_type >= 0)
      EXPECT_EQ(expected_payload_type, encoded_info.payload_type);
    if (expected_send_even_if_empty >= 0)
      EXPECT_EQ(static_cast<bool>(expected_send_even_if_empty),
                encoded_info.send_even_if_empty);
  }

  WebRtcACMCodecParams acm_codec_params_;
  rtc::scoped_ptr<ACMGenericCodec> codec_;
  uint32_t timestamp_;
};

// This test verifies that CNG frames are delivered as expected. Since the frame
// size is set to 20 ms, we expect the first encode call to produce no output
// (which is signaled as 0 bytes output of type kNoEncoding). The next encode
// call should produce one SID frame of 9 bytes. The third call should not
// result in any output (just like the first one). The fourth and final encode
// call should produce an "empty frame", which is like no output, but with the
// return value from the encode function being 1. (The reason to produce an
// empty frame is to drive sending of DTMF packets in the RTP/RTCP module.)
TEST_F(AcmGenericCodecTest, VerifyCngFrames) {
  CreateCodec();
  uint32_t expected_timestamp = timestamp_;
  // Verify no frame.
  {
    SCOPED_TRACE("First encoding");
    EncodeAndVerify(0, 0, expected_timestamp, kNoEncoding, -1, -1);
  }

  // Verify SID frame delivered.
  {
    SCOPED_TRACE("Second encoding");
    EncodeAndVerify(9, 9, expected_timestamp, kPassiveDTXNB, kCngPt, 1);
  }

  // Verify no frame.
  {
    SCOPED_TRACE("Third encoding");
    EncodeAndVerify(0, 0, expected_timestamp, kNoEncoding, -1, -1);
  }

  // Verify NoEncoding.
  expected_timestamp += 2 * kDataLengthSamples;
  {
    SCOPED_TRACE("Fourth encoding");
    EncodeAndVerify(1, 0, expected_timestamp, kNoEncoding, kCngPt, 1);
  }
}

}  // namespace acm2
}  // namespace webrtc
