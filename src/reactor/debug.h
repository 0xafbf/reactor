#pragma once

#include "log.h"



#if defined(__has_builtin) && !defined(__ibmxl__)
#  if __has_builtin(__builtin_debugtrap)
#    define rBreak() __builtin_debugtrap()
#  elif __has_builtin(__debugbreak)
#    define rBreak() __debugbreak()
#  endif
#endif
#if !defined(rBreak)
#  if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#    define rBreak() __debugbreak()
#  elif defined(__ARMCC_VERSION)
#    define rBreak() __breakpoint(42)
#  elif defined(__ibmxl__) || defined(__xlC__)
#    include <builtins.h>
#    define rBreak() __trap(42)
#  elif defined(__DMC__) && defined(_M_IX86)
	static inline void rBreak(void) { __asm int 3h; }
#  elif defined(__i386__) || defined(__x86_64__)
	__attribute__((always_inline)) static inline void rBreak(void) { __asm__ __volatile__("int $03"); }
#  elif defined(__thumb__)
	static inline void rBreak(void) { __asm__ __volatile__(".inst 0xde01"); }
#  elif defined(__aarch64__)
	static inline void rBreak(void) { __asm__ __volatile__(".inst 0xd4200000"); }
#  elif defined(__arm__)
	static inline void rBreak(void) { __asm__ __volatile__(".inst 0xe7f001f0"); }
#  elif defined (__alpha__) && !defined(__osf__)
	static inline void rBreak(void) { __asm__ __volatile__("bpt"); }
#  elif defined(_54_)
	static inline void rBreak(void) { __asm__ __volatile__("ESTOP"); }
#  elif defined(_55_)
	static inline void rBreak(void) { __asm__ __volatile__(";\n .if (.MNEMONIC)\n ESTOP_1\n .else\n ESTOP_1()\n .endif\n NOP"); }
#  elif defined(_64P_)
	static inline void rBreak(void) { __asm__ __volatile__("SWBP 0"); }
#  elif defined(_6x_)
	static inline void rBreak(void) { __asm__ __volatile__("NOP\n .word 0x10000000"); }
#  elif defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 0) && defined(__GNUC__)
#    define rBreak() __builtin_trap()
#  else
#    include <signal.h>
#    if defined(SIGTRAP)
#      define rBreak() raise(SIGTRAP)
#    else
#      define rBreak() raise(SIGABRT)
#    endif
#  endif
#endif
// got this code from here: https://github.com/nemequ/portable-snippets/tree/master/debug-trap


#define CHECK(cond) {if(!(cond)){ERROR("\nAssertion Failed: %s %s:%d", #cond, __FILE__, __LINE__);rBreak();}}


// maybe move these to another place later
#define VK_CHECK(op) CHECK(op == VK_SUCCESS)
#define SPV_CHECK(op) CHECK(op == SPV_REFLECT_RESULT_SUCCESS)
