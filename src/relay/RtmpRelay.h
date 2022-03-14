//
// Created by user on 13/03/2022.
//

#ifndef MYPROJECT_RTMPRELAY_H
#define MYPROJECT_RTMPRELAY_H

#include <gst/gst.h>

#include <spdlog/spdlog.h>

class RtmpRelay
{
public:
  void initialize(const std::string &input_file, const std::string &output_url);
  void cleanup();
  void play();
  void wait();

  void update_text_overlay(const std::string &val);

private:

  GstElement *_pipeline = nullptr;
  GstElement *_source = nullptr;
  GstElement *_sink = nullptr;
  GstElement *_clockoverlay = nullptr;
  GstElement *_identity = nullptr;

  GstElement *_qtdemux  = nullptr;
  GstElement *_h264parse  = nullptr;
  GstElement *_flvmux = nullptr;
  GstElement *_rtmpsink = nullptr;

  GstElement *_decodebin = nullptr;
  GstElement *_videoconvert = nullptr;
  GstElement *_x264enc = nullptr;

  GstElement *_textoverlay = nullptr;
};


#endif// MYPROJECT_RTMPRELAY_H
