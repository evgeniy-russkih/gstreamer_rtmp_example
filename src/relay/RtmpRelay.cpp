#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-vararg"
//
// Created by user on 13/03/2022.
//

#include "RtmpRelay.h"

static void pts_analysis_cb(GstElement *_identity, GstBuffer *buffer, gpointer user_data)
{
  GstClockTime pts = GST_BUFFER_PTS(buffer);
  auto *instance = (RtmpRelay *)user_data;
  instance->update_text_overlay(std::to_string(pts));
}

static void cb_new_pad(GstElement *element, GstPad *pad, gpointer data)
{
  gchar *name;
  GstElement *other = static_cast<GstElement *>(data);

  name = gst_pad_get_name(pad);
  spdlog::info("A new pad %s was created for %s\n", name, gst_element_get_name(element));
  g_free(name);

  spdlog::info("element %s will be linked to %s\n", gst_element_get_name(element), gst_element_get_name(other));
  gst_element_link(element, other);
}


void RtmpRelay::initialize(const std::string &input_file, const std::string &output_url)
{
  /* Initialize GStreamer */
  gst_init(nullptr, nullptr);

  /* Create the elements */
  _source = gst_element_factory_make("filesrc", "source");
  _qtdemux = gst_element_factory_make("qtdemux", "qtdemux");
  _h264parse = gst_element_factory_make("h264parse", "h264parse");
  _flvmux = gst_element_factory_make("flvmux", "flvmux");

  _clockoverlay = gst_element_factory_make("clockoverlay", "clockoverlay");
  _identity = gst_element_factory_make("identity", "identity");
  _textoverlay = gst_element_factory_make("textoverlay", "textoverlay");

  _decodebin = gst_element_factory_make("decodebin", "decodebin");
  _videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
  _x264enc = gst_element_factory_make("x264enc", "x264enc");

  _rtmpsink = gst_element_factory_make("rtmpsink", "rtmpsink");

  /* Create the empty pipeline */
  _pipeline = gst_pipeline_new("test-pipeline");

  if (!_pipeline || !_source || !_qtdemux || !_h264parse || !_flvmux || !_rtmpsink || !_clockoverlay || !_decodebin
      || !_videoconvert || !_x264enc) {
    std::string error = "Not created.\n";
    spdlog::error(error);
    throw std::runtime_error(error);
  }

  gst_bin_add_many(GST_BIN(_pipeline),
    _source,
    _qtdemux,
    _decodebin,
    _videoconvert,
    _clockoverlay,
    _x264enc,
    _h264parse,
    _flvmux,
    _rtmpsink,
    _identity,
    _textoverlay,
    NULL);

  if (gst_element_link(_source, _qtdemux) != TRUE) {
    std::string error = "source link error.\n";
    spdlog::error(error);
    gst_object_unref(_pipeline);
    throw std::runtime_error(error);
  }

  if (gst_element_link_many(
        _videoconvert, _identity, _textoverlay, _clockoverlay, _x264enc, _h264parse, _flvmux, _rtmpsink, NULL)
      != TRUE) {
    std::string error = "rest link error.\n";
    spdlog::error(error);
    gst_object_unref(_pipeline);
    throw std::runtime_error(error);
  }

  /* Modify properties */
  g_object_set(_source, "location", input_file.c_str(), NULL);

  g_object_set(_rtmpsink, "location", output_url.c_str(), NULL);
  g_object_set(_textoverlay, "text", "", NULL);

  g_signal_connect_data(_identity, "handoff", G_CALLBACK(pts_analysis_cb), (void *)this, NULL, GConnectFlags());
  g_signal_connect(_qtdemux, "pad-added", G_CALLBACK(cb_new_pad), _decodebin);
  g_signal_connect(_decodebin, "pad-added", G_CALLBACK(cb_new_pad), _videoconvert);
}

void RtmpRelay::play()
{
  GstStateChangeReturn ret;

  /* Start playing */
  spdlog::info("start playing");
  ret = gst_element_set_state(_pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    std::string error = "Unable to set the pipeline to the playing state.\n";
    spdlog::error(error);
    gst_object_unref(_pipeline);
    throw std::runtime_error(error);
  }
}


void RtmpRelay::cleanup()
{
  gst_element_set_state(_pipeline, GST_STATE_NULL);
  gst_object_unref(_pipeline);
}

void RtmpRelay::wait()
{
  spdlog::info("waiting");
  GstBus *bus;
  GstMessage *msg;

  /* Wait until error or EOS */
  bus = gst_element_get_bus(_pipeline);
  msg = gst_bus_timed_pop_filtered(
    bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  /* Parse message */
  if (msg != NULL) {
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error(msg, &err, &debug_info);
      spdlog::error("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
      spdlog::error("Debugging information: %s\n", debug_info ? debug_info : "none");
      g_clear_error(&err);
      g_free(debug_info);
      throw std::exception();
      break;
    case GST_MESSAGE_EOS:
      spdlog::info("End-Of-Stream reached.\n");
      if (gst_element_seek(_pipeline,
            1.0,
            GST_FORMAT_TIME,
            GST_SEEK_FLAG_FLUSH,
            GST_SEEK_TYPE_SET,
            0,
            GST_SEEK_TYPE_NONE,
            GST_CLOCK_TIME_NONE)
          != TRUE) {
        std::string error = "seek failed\n";
        spdlog::info(error);
        throw std::runtime_error(error);
      }

      break;
    default:
      /* We should not reach here because we only asked for ERRORs and EOS */
      spdlog::error("Unexpected message received.\n");
      throw std::exception();
      break;
    }
    gst_message_unref(msg);
  }

  /* Free resources */
  gst_object_unref(bus);
}

void RtmpRelay::update_text_overlay(const std::string &val) { g_object_set(_textoverlay, "text", val.c_str(), NULL); }


#pragma clang diagnostic pop