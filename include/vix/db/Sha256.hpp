/**
 *
 *  @file Sha256.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_DB_SHA256_HPP
#define VIX_DB_SHA256_HPP

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace vix::db
{
  /**
   * @brief Incremental SHA-256 hash calculator.
   *
   * Sha256 provides a minimal, self-contained implementation of the
   * SHA-256 hashing algorithm. It supports incremental updates and
   * produces a 256-bit digest.
   *
   * This utility is primarily intended for internal database-related
   * features such as migration checksums.
   */
  class Sha256
  {
  public:
    /**
     * @brief Construct a new SHA-256 hasher.
     *
     * The hasher is initialized in a reset state.
     */
    Sha256() { reset(); }

    /**
     * @brief Reset the internal state.
     *
     * After calling reset(), the hasher can be reused to compute
     * a new digest.
     */
    void reset();

    /**
     * @brief Update the hash with raw binary data.
     *
     * @param data Pointer to the input buffer.
     * @param len  Number of bytes to process.
     */
    void update(const void *data, std::size_t len);

    /**
     * @brief Update the hash with string data.
     *
     * @param s Input string view.
     */
    void update(std::string_view s) { update(s.data(), s.size()); }

    /**
     * @brief Finalize the hash and return the digest.
     *
     * Calling this function finalizes the computation and returns
     * the 32-byte SHA-256 digest.
     *
     * @return SHA-256 digest.
     */
    std::array<std::uint8_t, 32> digest();

    /**
     * @brief Convert a SHA-256 digest to a hexadecimal string.
     *
     * @param d Digest bytes.
     * @return Lowercase hexadecimal representation.
     */
    static std::string hex(std::array<std::uint8_t, 32> d);

  private:
    /**
     * @brief Process a single 512-bit message block.
     *
     * @param block Input block.
     */
    void transform(const std::uint8_t block[64]);

    /// Total number of processed bits
    std::uint64_t bitlen_ = 0;

    /// Internal hash state
    std::array<std::uint32_t, 8> state_{};

    /// Partial message buffer
    std::array<std::uint8_t, 64> buffer_{};

    /// Number of bytes currently stored in the buffer
    std::size_t buffer_len_ = 0;
  };

  /**
   * @brief Compute a SHA-256 hash and return it as hexadecimal.
   *
   * Convenience helper for hashing a single string.
   *
   * @param s Input string.
   * @return SHA-256 digest encoded as hexadecimal.
   */
  inline std::string sha256_hex(std::string_view s)
  {
    Sha256 h;
    h.update(s);
    return Sha256::hex(h.digest());
  }

} // namespace vix::db

#endif // VIX_DB_SHA256_HPP
