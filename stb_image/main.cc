// Copyright 2024 Michael Carroll

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <string>
#include <sstream>

struct ImageInfo
{
  std::string filename;
  int width {-1};
  int height {-1};
  int numChannels {1};

  bool valid {false};
  bool isHdr {false};
  bool is16bit {false};

  explicit ImageInfo(std::string _filename):
    filename(std::move(_filename))
  {
    auto ok = stbi_info(this->filename.c_str(), &this->width, &this->height, &this->numChannels);
    this->valid = (ok == 1);

    auto is_hdr = stbi_is_hdr(this->filename.c_str());
    this->isHdr = (is_hdr == 1);

    auto is_16bit = stbi_is_16_bit(filename.c_str());
    this->is16bit = (is_16bit == 1);
  }

  [[nodiscard]] std::string toString() const
  {
    std::stringstream ss;
    ss << "Filename: " << this->filename << " ";
    if (this->valid && this->isHdr && this->is16bit)
    {
      ss << " [valid,HDR,16bit]";
    } else if (this->valid && this->isHdr) {
      ss << " [valid,HDR]";
    } else if (this->valid && this->is16bit) {
      ss << " [valid,16bit]";
    } else if (this->valid) {
      ss << " [valid]";
    }

    if (this->valid)
    {
      ss << "[" << this->width << "x" << this->height << "]";
      ss << "[";
      switch (this->numChannels)
      {
        case 1:
          ss << "grey";
          break;
        case 2:
          ss << "grey,alpha";
          break;
        case 3:
          ss << "red,green,blue";
          break;
        case 4:
          ss << "red,green,blue,alpha";
          break;
      }
      ss << "]";
    }
    return ss.str();
  }
};

struct Image
{
  ImageInfo info;
  uint8_t *data {nullptr};
  uint16_t *data_16bit {nullptr};
  float *data_hdr {nullptr};

  explicit Image(std::string filename):
    info(std::move(filename))
  {
    if (info.valid && info.is16bit)
    {
      data_16bit = stbi_load_16(info.filename.c_str(), &info.width, &info.height, &info.numChannels, 0);
    } else if (info.valid && info.isHdr) {
      data_hdr = stbi_loadf(info.filename.c_str(), &info.width, &info.height, &info.numChannels, 0);
    } else if (info.valid) {
      data = stbi_load(info.filename.c_str(), &info.width, &info.height, &info.numChannels, 0);
    }
  }

  ~Image()
  {
    stbi_image_free(this->data);
  }
};


int main(int argc, char **argv)
{
  if (argc <= 1)
    return -1;


  for (auto ii = 1; ii < argc; ++ii)
  {
    auto image = Image(argv[ii]);
    std::cout << image.info.toString() << std::endl;
  }
}
