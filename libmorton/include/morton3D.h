// Libmorton - Methods to encode/decode 64-bit morton codes from/to 32-bit (x,y,z) coordinates
#ifndef MORTON3D_H_
#define MORTON3D_H_

#include <stdint.h>
#include "morton3D_lookup_tables.h"

#if _MSC_VER
#include <intrin.h>
#endif

using namespace std;

//#define LIBMORTON_USE_INTRINSICS

// THESE DEFAULT METHODS WILL ALWAYS POINT TO THE FASTEST IMPLEMENTED METHOD
// -------------------------------------------------------------------------
// ENCODE a 32-bit (x,y,z) coordinate to a 64-bit morton code
inline uint_fast64_t morton3D_64_Encode(const uint_fast32_t x, const uint_fast32_t y, const uint_fast32_t z);
// DECODE a 64-bit morton code into 32-bit (x,y,z) coordinates (use this if you need all three coordinates)
inline uint_fast64_t morton3D_64_Decode(const uint_fast64_t morton, uint_fast32_t& x, uint_fast32_t& y, uint_fast32_t& z);
// DECODE a 64-bit morton code into a 32-bit coordinate (use this if you only need x, y or z)
inline uint_fast32_t morton3D_64_Decode_X(const uint_fast64_t morton);
inline uint_fast32_t morton3D_64_Decode_Y(const uint_fast64_t morton);
inline uint_fast32_t morton3D_64_Decode_Z(const uint_fast64_t morton);

// encoding with lookup table
inline uint_fast64_t morton3D_32_Encode_LUT(const uint_fast16_t x, const uint_fast16_t y, const uint_fast16_t z){
	uint_fast64_t answer =
		Morton3D_64_encode_z_256[(z >> 16) & 0xFF] |
		Morton3D_64_encode_y_256[(y >> 16) & 0xFF] |
		Morton3D_64_encode_x_256[(x >> 16) & 0xFF];
	answer = answer << 48 |
		Morton3D_64_encode_z_256[(z >> 8) & 0xFF] |
		Morton3D_64_encode_y_256[(y >> 8) & 0xFF] |
		Morton3D_64_encode_x_256[(x >> 8) & 0xFF];
	answer = answer << 24 |
		Morton3D_64_encode_z_256[(z)& 0xFF] |
		Morton3D_64_encode_y_256[(y)& 0xFF] |
		Morton3D_64_encode_x_256[(x)& 0xFF];
	return answer;
}

// encoding with lookup table
inline uint_fast64_t morton3D_64_Encode_LUT_shifted(const uint_fast32_t x, const uint_fast32_t y, const uint_fast32_t z){
	uint_fast64_t answer =
		Morton3D_64_encode_z_256[(z >> 16) & 0xFF] |
		Morton3D_64_encode_y_256[(y >> 16) & 0xFF] |
		Morton3D_64_encode_x_256[(x >> 16) & 0xFF];
	answer = answer << 48 |
		Morton3D_64_encode_z_256[(z >> 8) & 0xFF] |
		Morton3D_64_encode_y_256[(y >> 8) & 0xFF] |
		Morton3D_64_encode_x_256[(x >> 8) & 0xFF];
	answer = answer << 24 |
		Morton3D_64_encode_z_256[z & 0xFF] |
		Morton3D_64_encode_y_256[y & 0xFF] |
		Morton3D_64_encode_x_256[x & 0xFF];
	return answer;
}

// encoding with lookup table
inline uint_fast64_t morton3D_64_Encode_LUT(const uint_fast32_t x, const uint_fast32_t y, const uint_fast32_t z){
	uint_fast64_t answer =
		 (Morton3D_64_encode_x_256[(z >> 16) & 0xFF] << 2)
		|(Morton3D_64_encode_x_256[(y >> 16) & 0xFF] << 1) 
		|Morton3D_64_encode_x_256[(x >> 16) & 0xFF];
	answer = answer << 48 | 
		 (Morton3D_64_encode_x_256[(z >> 8) & 0xFF] << 2)
		|(Morton3D_64_encode_x_256[(y >> 8) & 0xFF] << 1)
		|Morton3D_64_encode_x_256[(x >> 8) & 0xFF];
	answer = answer << 24 |
		 (Morton3D_64_encode_x_256[z & 0xFF] << 2)
		|(Morton3D_64_encode_x_256[y & 0xFF] << 1)
		|Morton3D_64_encode_x_256[x & 0xFF];
	return answer;
}

inline void morton3D_64_Decode_LUT(const uint_fast64_t morton, uint_fast32_t& x, uint_fast32_t& y, uint_fast32_t& z){
	x = 0; y = 0; z = 0;
#ifdef LIBMORTON_USE_INTRINSICS
	// For Microsoft compilers use _BitScanForward & _BitScanReverse.
	// For GCC use __builtin_ffs, __builtin_clz, __builtin_ctz.

	// use bit manipulation intrinsic to find out first bit, for early termination
	unsigned long firstbit_location;
#if _MSC_VER
	// are the casts necessary? and the blanking in the second one?
	// Does the pragma stop the compiler from optimizing this?
	// is it cheaper to do this using the 64 bit one
	if (_BitScanReverse(&firstbit_location, (morton >> 32))){ // check first part of morton code
		firstbit_location += 32;
	} else if ( ! _BitScanReverse(&firstbit_location, (morton & 0xFFFFFFFF))){ // also test last part of morton code
		return;
	}
#endif
	x = x | Morton3D_64_decode_x_512[morton & 0x1ff];
	y = y | Morton3D_64_decode_y_512[morton & 0x1ff];
	z = z | Morton3D_64_decode_z_512[morton & 0x1ff];
	if (firstbit_location < 9) return;
	x = x | (Morton3D_64_decode_x_512[((morton >> 9) & 0x1ff)] << 3);
	y = y | (Morton3D_64_decode_y_512[((morton >> 9) & 0x1ff)] << 3);
	z = z | (Morton3D_64_decode_z_512[((morton >> 9) & 0x1ff)] << 3);
	if (firstbit_location < 18) return;
	x = x | (Morton3D_64_decode_x_512[((morton >> 18) & 0x1ff)] << 6);
	y = y | (Morton3D_64_decode_y_512[((morton >> 18) & 0x1ff)] << 6);
	z = z | (Morton3D_64_decode_z_512[((morton >> 18) & 0x1ff)] << 6);
	if (firstbit_location < 27) return;
	x = x | (Morton3D_64_decode_x_512[((morton >> 27) & 0x1ff)] << 9);
	y = y | (Morton3D_64_decode_y_512[((morton >> 27) & 0x1ff)] << 9);
	z = z | (Morton3D_64_decode_z_512[((morton >> 27) & 0x1ff)] << 9);
	if (firstbit_location < 36) return;
	x = x | (Morton3D_64_decode_x_512[((morton >> 36) & 0x1ff)] << 12);
	y = y | (Morton3D_64_decode_y_512[((morton >> 36) & 0x1ff)] << 12);
	z = z | (Morton3D_64_decode_z_512[((morton >> 36) & 0x1ff)] << 12);
	if (firstbit_location < 46) return;
	x = x | (Morton3D_64_decode_x_512[((morton >> 46) & 0x1ff)] << 15);
	y = y | (Morton3D_64_decode_y_512[((morton >> 46) & 0x1ff)] << 15);
	z = z | (Morton3D_64_decode_z_512[((morton >> 46) & 0x1ff)] << 15);
	if (firstbit_location < 54) return;
	x = x | (Morton3D_64_decode_x_512[((morton >> 54) & 0x1ff)] << 18);
	y = y | (Morton3D_64_decode_y_512[((morton >> 54) & 0x1ff)] << 18);
	z = z | (Morton3D_64_decode_z_512[((morton >> 54) & 0x1ff)] << 18);
	return;
#else
	// standard portable version
	x = 0 | Morton3D_64_decode_x_512[morton & 0x1ff]
		| (Morton3D_64_decode_x_512[((morton >> 9) & 0x1ff)] << 3)
		| (Morton3D_64_decode_x_512[((morton >> 18) & 0x1ff)] << 6)
		| (Morton3D_64_decode_x_512[((morton >> 27) & 0x1ff)] << 9)
		| (Morton3D_64_decode_x_512[((morton >> 36) & 0x1ff)] << 12)
		| (Morton3D_64_decode_x_512[((morton >> 46) & 0x1ff)] << 15)
		| (Morton3D_64_decode_x_512[((morton >> 54) & 0x1ff)] << 18);
	y = 0 | Morton3D_64_decode_y_512[morton & 0x1ff]
		| (Morton3D_64_decode_y_512[((morton >> 9) & 0x1ff)] << 3)
		| (Morton3D_64_decode_y_512[((morton >> 18) & 0x1ff)] << 6)
		| (Morton3D_64_decode_y_512[((morton >> 27) & 0x1ff)] << 9)
		| (Morton3D_64_decode_y_512[((morton >> 36) & 0x1ff)] << 12)
		| (Morton3D_64_decode_y_512[((morton >> 46) & 0x1ff)] << 15)
		| (Morton3D_64_decode_y_512[((morton >> 54) & 0x1ff)] << 18);
	z = 0 | Morton3D_64_decode_z_512[morton & 0x1ff]
		| (Morton3D_64_decode_z_512[((morton >> 9) & 0x1ff)] << 3)
		| (Morton3D_64_decode_z_512[((morton >> 18) & 0x1ff)] << 6)
		| (Morton3D_64_decode_z_512[((morton >> 27) & 0x1ff)] << 9)
		| (Morton3D_64_decode_z_512[((morton >> 36) & 0x1ff)] << 12)
		| (Morton3D_64_decode_z_512[((morton >> 46) & 0x1ff)] << 15)
		| (Morton3D_64_decode_z_512[((morton >> 54) & 0x1ff)] << 18);
#endif
}

inline uint_fast32_t morton3D_64_Decode_X_LUT(const uint_fast64_t morton){
	return 0 | Morton3D_64_decode_x_512[morton & 0x1ff]
		| (Morton3D_64_decode_x_512[((morton >> 9) & 0x1ff)] << 3)
		| (Morton3D_64_decode_x_512[((morton >> 18) & 0x1ff)] << 6)
		| (Morton3D_64_decode_x_512[((morton >> 27) & 0x1ff)] << 9)
		| (Morton3D_64_decode_x_512[((morton >> 36) & 0x1ff)] << 12)
		| (Morton3D_64_decode_x_512[((morton >> 46) & 0x1ff)] << 15)
		| (Morton3D_64_decode_x_512[((morton >> 54) & 0x1ff)] << 18);
}

inline uint_fast32_t morton3D_64_Decode_Y_LUT(const uint_fast64_t morton){
	return 0 | Morton3D_64_decode_y_512[morton & 0x1ff]
		| (Morton3D_64_decode_y_512[((morton >> 9) & 0x1ff)] << 3)
		| (Morton3D_64_decode_y_512[((morton >> 18) & 0x1ff)] << 6)
		| (Morton3D_64_decode_y_512[((morton >> 27) & 0x1ff)] << 9)
		| (Morton3D_64_decode_y_512[((morton >> 36) & 0x1ff)] << 12)
		| (Morton3D_64_decode_y_512[((morton >> 46) & 0x1ff)] << 15)
		| (Morton3D_64_decode_y_512[((morton >> 54) & 0x1ff)] << 18);
}

inline uint_fast32_t morton3D_64_Decode_Z_LUT(const uint_fast64_t morton){
	return 0 | Morton3D_64_decode_z_512[morton & 0x1ff]
		| (Morton3D_64_decode_z_512[((morton >> 9) & 0x1ff)] << 3)
		| (Morton3D_64_decode_z_512[((morton >> 18) & 0x1ff)] << 6)
		| (Morton3D_64_decode_z_512[((morton >> 27) & 0x1ff)] << 9)
		| (Morton3D_64_decode_z_512[((morton >> 36) & 0x1ff)] << 12)
		| (Morton3D_64_decode_z_512[((morton >> 46) & 0x1ff)] << 15)
		| (Morton3D_64_decode_z_512[((morton >> 54) & 0x1ff)] << 18);
}

// define default methods
inline uint_fast64_t morton3D_64_Encode(const uint_fast32_t x, const uint_fast32_t y, const uint_fast32_t z){
	return morton3D_64_Encode_LUT_shifted(x, y, z);
}

inline uint_fast64_t morton3D_64_Decode(const uint_fast64_t morton, uint_fast32_t& x, uint_fast32_t& y, uint_fast32_t& z){
	morton3D_64_Decode_LUT(morton, x, y, z);
}

inline uint_fast32_t morton3D_64_Decode_X(const uint_fast64_t morton){
	return morton3D_64_Decode_X_LUT(morton);
}

inline uint_fast32_t morton3D_64_Decode_Y(const uint_fast64_t morton){
	return morton3D_64_Decode_Y_LUT(morton);
}

inline uint_fast32_t morton3D_64_Decode_Z(const uint_fast64_t morton){
	return morton3D_64_Decode_Z_LUT(morton);
}

#endif // MORTON3D_H_