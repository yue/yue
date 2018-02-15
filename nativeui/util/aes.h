// This file is published under public domain.
// Originially from https://github.com/kokke/tiny-AES-c.

#ifndef NATIVEUI_UTIL_AES_H_
#define NATIVEUI_UTIL_AES_H_

#include <stdint.h>

#include <string>

#define AES128 1
#define AES_BLOCKLEN 16

#if defined(AES256) && (AES256 == 1)
  #define AES_KEYLEN 32
  #define AES_KEYEXPSIZE 240
#elif defined(AES192) && (AES192 == 1)
  #define AES_KEYLEN 24
  #define AES_KEYEXPSIZE 208
#else
  #define AES_KEYLEN 16
  #define AES_KEYEXPSIZE 176
#endif

namespace nu {

class AES {
 public:
  bool Init(const std::string& key, const std::string& iv);
  bool IsValid() const { return is_valid_; }

  void CBCEncryptBuffer(uint8_t* buf, uint32_t len);
  void CBCDecryptBuffer(uint8_t* buf, uint32_t len);

 private:
  bool is_valid_ = false;

  uint8_t round_key_[AES_KEYEXPSIZE];
  uint8_t iv_[AES_BLOCKLEN];
};

}  // namespace nu

#endif  // NATIVEUI_UTIL_AES_H_
