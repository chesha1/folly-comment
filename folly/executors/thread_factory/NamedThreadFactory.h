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

#include <atomic>
#include <string>
#include <thread>

#include <folly/Conv.h>
#include <folly/Range.h>
#include <folly/executors/thread_factory/ThreadFactory.h>
#include <folly/system/ThreadName.h>

namespace folly {
    // 创建具有特定命名的新线程
    class NamedThreadFactory : public ThreadFactory {
    public:
        // 构造函数，初始化线程名的前缀和后缀
        explicit NamedThreadFactory(folly::StringPiece prefix)
                : prefix_(prefix.str()), suffix_(0) {}

        std::thread newThread(Func &&func) override {
            // 生成线程名称，后缀递增
            auto name = folly::to<std::string>(prefix_, suffix_++);
            return std::thread(
                    [func = std::move(func), name = std::move(name)]() mutable {
                        folly::setThreadName(name);
                        func();
                    });
        }

        // 设置线程名的前缀
        void setNamePrefix(folly::StringPiece prefix) { prefix_ = prefix.str(); }

        // 获取线程名的前缀
        const std::string &getNamePrefix() const override { return prefix_; }

    protected:
        std::string prefix_;

        // 用于生成线程名的后缀，是一个原子类型，保证并发安全
        std::atomic<uint64_t> suffix_;
    };

} // namespace folly
