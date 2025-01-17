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

#include <glog/logging.h>

#include <folly/Executor.h>
#include <folly/MPMCQueue.h>
#include <folly/Range.h>
#include <folly/executors/task_queue/BlockingQueue.h>
#include <folly/synchronization/LifoSem.h>

namespace folly {

    // 支持优先级的后进先出的多生产者多消费者队列
    // 主要功能:
    // 支持多个优先级,优先级越大越先出队。
    // 使用 LifoSem 作为信号量,实现后进先出。
    // 内部包含多个MPMCQueue,每个队列对应一个优先级。
    // 提供添加和取出的接口,可指定优先级。
    // 添加时若队列满可以抛异常或阻塞。
    // 取出时为空可以等待或立即返回。
    // 支持非阻塞的取出尝试。
    // 可指定每个优先级的队列容量。
    template<
            class T,
            QueueBehaviorIfFull kBehavior = QueueBehaviorIfFull::THROW,
            class Semaphore = folly::LifoSem>
    class PriorityLifoSemMPMCQueue : public BlockingQueue<T> {
    public:
        // Note A: The queue pre-allocates all memory for max_capacity
        // Note B: To use folly::Executor::*_PRI, for numPriorities == 2
        //         MID_PRI and HI_PRI are treated at the same priority level.
        PriorityLifoSemMPMCQueue(
                uint8_t numPriorities,
                size_t max_capacity,
                const typename Semaphore::Options &semaphoreOptions = {})
                : sem_(semaphoreOptions) {
            CHECK_GT(numPriorities, 0) << "Number of priorities should be positive";
            queues_.reserve(numPriorities);
            for (int8_t i = 0; i < numPriorities; i++) {
                queues_.emplace_back(max_capacity);
            }
        }

        PriorityLifoSemMPMCQueue(
                folly::Range<const size_t *> capacities,
                const typename Semaphore::Options &semaphoreOptions = {})
                : sem_(semaphoreOptions) {
            CHECK_GT(capacities.size(), 0) << "Number of priorities should be positive";
            CHECK_LT(capacities.size(), 256) << "At most 255 priorities supported";

            queues_.reserve(capacities.size());
            for (auto capacity: capacities) {
                queues_.emplace_back(capacity);
            }
        }

        uint8_t getNumPriorities() override { return queues_.size(); }

        // Add at medium priority by default
        BlockingQueueAddResult add(T item) override {
            return addWithPriority(std::move(item), folly::Executor::MID_PRI);
        }

        BlockingQueueAddResult addWithPriority(T item, int8_t priority) override {
            int mid = getNumPriorities() / 2;
            size_t queue = priority < 0
                           ? std::max(0, mid + priority)
                           : std::min(getNumPriorities() - 1, mid + priority);
            CHECK_LT(queue, queues_.size());
            switch (kBehavior) { // static
                case QueueBehaviorIfFull::THROW:
                    if (!queues_[queue].writeIfNotFull(std::move(item))) {
                        throw QueueFullException("LifoSemMPMCQueue full, can't add item");
                    }
                    break;
                case QueueBehaviorIfFull::BLOCK:
                    queues_[queue].blockingWrite(std::move(item));
                    break;
            }
            return sem_.post();
        }

        T take() override {
            T item;
            while (true) {
                if (nonBlockingTake(item)) {
                    return item;
                }
                sem_.wait();
            }
        }

        folly::Optional<T> try_take_for(std::chrono::milliseconds time) override {
            T item;
            while (true) {
                if (nonBlockingTake(item)) {
                    return item;
                }
                if (!sem_.try_wait_for(time)) {
                    return folly::none;
                }
            }
        }

        bool nonBlockingTake(T &item) {
            for (auto it = queues_.rbegin(); it != queues_.rend(); it++) {
                if (it->readIfNotEmpty(item)) {
                    return true;
                }
            }
            return false;
        }

        size_t size() override {
            size_t size = 0;
            for (auto &q: queues_) {
                size += q.size();
            }
            return size;
        }

        size_t sizeGuess() const {
            size_t size = 0;
            for (auto &q: queues_) {
                size += q.sizeGuess();
            }
            return size;
        }

    private:
        Semaphore sem_;
        std::vector<folly::MPMCQueue<T>> queues_;
    };

} // namespace folly
