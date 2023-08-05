/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/Portability.h>

#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace folly {
    // 定义了一个编译器屏障，防止编译器重新排序读取或写入内存的指令
    inline void asm_volatile_memory() {
        // __GNUC__：这个宏在GNU C编译器（GCC）中定义。如果代码正在使用GCC进行编译，这个宏将被定义
        // 下面这句的意思是：“如果代码正在使用GCC或Clang编译器进行编译，则执行以下代码块。”
#if defined(__GNUC__) || defined(__clang__)
        // GCC和Clang编译器中的内联汇编指令，用于创建一个编译器内存屏障（compiler memory barrier）
        // asm：这告诉编译器下面是一条内联汇编指令。
        // volatile：这个关键字指示编译器不要重新排序或优化掉这个汇编指令

        // ""：这是内联汇编代码本身的部分，但在这里它是空的。通常，您会在这里放入实际的汇编指令，
        // 但在这种情况下，您只想创建一个编译器屏障，所以不需要任何实际的汇编代码

        // : : : "memory"：这个部分是内联汇编的约束部分。
        // 最后的"memory"告诉编译器这个指令将会影响内存。
        // 这意味着在这个指令之前和之后的任何内存访问不能被重新排序
        asm volatile("" : : : "memory");
#elif defined(_MSC_VER)
        ::_ReadWriteBarrier();
#endif
    }

    inline void asm_volatile_pause() {
        // 这个函数用于插入一个"暂停"指令，通常用于繁忙等待循环中以减少功耗和执行资源的竞争
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
        ::_mm_pause();
#elif defined(__i386__) || FOLLY_X64 || \
    (defined(__mips_isa_rev) && __mips_isa_rev > 1)
        asm volatile("pause");
#elif FOLLY_AARCH64
        asm volatile("isb");
#elif (defined(__arm__) && !(__ARM_ARCH < 7))
        asm volatile("yield");
#elif FOLLY_PPC64
        asm volatile("or 27,27,27");
#endif
    }
} // namespace folly
