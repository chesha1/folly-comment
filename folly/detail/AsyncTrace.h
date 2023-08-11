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

#include <chrono>

#include <folly/Optional.h>

// 包含了与异步操作跟踪相关的一些功能
// 各种 log* 函数：这些函数提供了记录与执行器和异步操作相关的事件的功能。
// 从其名称来看，它们可能会在某些事件发生时记录日志

namespace folly {
class Executor;
class IOExecutor;
namespace async_tracing {
enum class DiscardHasDeferred {
  NO_EXECUTOR,
  DEFERRED_EXECUTOR,
};
void logSetGlobalCPUExecutor(Executor*) noexcept;
void logSetGlobalCPUExecutorToImmutable() noexcept;
void logGetGlobalCPUExecutor(Executor*) noexcept;
void logGetImmutableCPUExecutor(Executor*) noexcept;
void logSetGlobalIOExecutor(IOExecutor*) noexcept;
void logGetGlobalIOExecutor(IOExecutor*) noexcept;
void logGetImmutableIOExecutor(IOExecutor*) noexcept;
void logSemiFutureVia(Executor*, Executor*) noexcept;
void logFutureVia(Executor*, Executor*) noexcept;
void logBlockingOperation(std::chrono::milliseconds) noexcept;
void logSemiFutureDiscard(
    DiscardHasDeferred /* hasDeferredExecutor */) noexcept;
} // namespace async_tracing
} // namespace folly
