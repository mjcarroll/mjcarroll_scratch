// Copyright 2024 Michael Carroll
// extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
};

#include <array>
#include <iostream>
#include <optional>
#include <vector>

void read(const std::string &_filename)
{
  int error = 0;
  AVCodecContext* codecCtx = nullptr;
  AVFormatContext* formatCtx = nullptr;
  const AVCodec * codec = nullptr;
  AVFrame* frame = nullptr;
  AVPacket* packet = nullptr;

  int streamIdx = -1;
  int numChannels = -1;
  int sampleRate = -1;

  // Helper function to read error string
  auto read_averr = [&error]() -> std::optional<std::string>{
    if (error != 0) {
      std::array<char, 200> errbuf = {};
      av_strerror(error, errbuf.data(), errbuf.size());
      return std::string(errbuf.data());
    }
    return {};
  };

  // Get format information
  error = avformat_open_input(&formatCtx, _filename.c_str(), nullptr, nullptr);
  if (auto strerr = read_averr())
  {
    std::cerr << "Error opening input: " << _filename << " " << strerr.value() << std::endl;
    goto cleanup;  // NOLINT
  }

  error = avformat_find_stream_info(formatCtx, nullptr);
  if (auto strerr = read_averr())
  {
    std::cerr << "Error finding stream info: " << _filename << " " << strerr.value() << std::endl;
    goto cleanup;  // NOLINT
  }

  // Find an audio stream and its decoder
  streamIdx = av_find_best_stream(
      formatCtx,
      AVMEDIA_TYPE_AUDIO,
      -1, -1, &codec, 0);

  if (streamIdx < 0)
  {
    std::cerr << "No audio stream found: " << _filename << std::endl;
    goto cleanup;  // NOLINT
  }

  codecCtx = avcodec_alloc_context3(codec);
  if (codecCtx == nullptr)
  {
    std::cerr << "Could not allocate codec context: " << _filename << std::endl;
    goto cleanup;  // NOLINT
  }

  error = avcodec_parameters_to_context(codecCtx, formatCtx->streams[streamIdx]->codecpar);
  if (auto strerr = read_averr())
  {
    std::cerr << "Error setting codec parameters: " << _filename << " " << strerr.value() << std::endl;
    goto cleanup;  // NOLINT
  }

  error = avcodec_open2(codecCtx, codec, nullptr);
  if (auto strerr = read_averr())
  {
    std::cerr << "Error opening codec: " << _filename << " " << strerr.value() << std::endl;
    goto cleanup;  // NOLINT
  }

  numChannels = codecCtx->ch_layout.nb_channels;
  if (numChannels <= 0)
  {
    std::cerr << "numChannels doesn't make sense: " << numChannels << std::endl;
    goto cleanup;  // NOLINT
  }

  sampleRate = codecCtx->sample_rate;
  if (sampleRate <= 0)
  {
    std::cerr << "sampleRate doesn't make sense: " << sampleRate << std::endl;
    goto cleanup;  // NOLINT
  }

  frame = av_frame_alloc();
  if (frame == nullptr) {
    std::cerr << "Failed to allocate frame" << std::endl;
    goto cleanup;  // NOLINT
  }

  packet = av_packet_alloc();
  if (packet == nullptr) {
    std::cerr << "Failed to allocate packet" << std::endl;
    goto cleanup;  //NOLINT
  }

  while (true)
  {
    error = av_read_frame(formatCtx, packet);
    if (error == AVERROR_EOF)
      break;

    if (auto strerr = read_averr())
    {
      std::cerr << "Error reading from file: " << _filename << " " << strerr.value() << std::endl;
      goto cleanup;  // NOLINT
    }

    if (packet->stream_index != streamIdx ) {
      continue;
    }

    error = avcodec_send_packet(codecCtx, packet);
    if (auto strerr = read_averr())
    {
      std::cerr << "Error sending packet: " << strerr.value() << std::endl;
      goto cleanup;  // NOLINT
    }

    while ((error = avcodec_receive_frame(codecCtx, frame)) == 0) {
      std::cout << "Got Frame: " << std::endl;
      std::cout << "Num Samples: " << frame->nb_samples << std::endl;
      std::cout << "Num channels: " << frame->ch_layout.nb_channels << std::endl;
      std::cout << "Bytes per sample: " << av_get_bytes_per_sample(codecCtx->sample_fmt) << std::endl;
      std::cout << "Linesize: " << frame->linesize[0] << " " <<  frame->linesize[1] << std::endl;
    }

    if (error != AVERROR(EAGAIN))
    {
      auto strerr = read_averr();
      std::cerr << "Error receiving packet from decoder: " << strerr.value() << std::endl;
      goto cleanup;
    }
  }

cleanup:
  av_packet_free(&packet);
  avcodec_close(codecCtx);
  avcodec_free_context(&codecCtx);
  avio_closep(&formatCtx->pb);
  avformat_free_context(formatCtx);
  av_frame_free(&frame);
}

int main(int argc, const char* argv[])
{
  if (argc == 1)
    return -1;

  read(argv[1]);
  return 0;
}
