//
// Created by therdel on 19.10.19.
//
#pragma once
#include <optional>
#include <thread>

#include "Utility.hpp"
#include "Pointers/Signatures.hpp"
#include "BoyerMooreSegmentScanner.hpp"
#include "MemoryUtils.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX
#include "Log.hpp"

class MemoryScanner final {
public:
    /// find occurances of a signature in the address space
    /// \returns matching locations after applying the signatures offset
    template<typename Scanner = BoyerMooreSegmentScanner>
    static auto scanSignature(const SignatureAOI& signatureAOI, std::optional<std::string_view> description = std::nullopt) -> std::vector<uintptr_t> {
        constexpr auto now = [] { return std::chrono::system_clock::now(); };
        auto start = now();

        Scanner scanner(signatureAOI);

        std::vector<uintptr_t> matches = _scan_parallel(scanner, signatureAOI);
        auto duration = now() - start;

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        if (description) {
            Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, *description, ": scan duration: ", ms, "ms");
        }

        /*          debug:  release:
         * regex:   627     83
         * linear:  92      32
         * boyer:   63       9
         */

        return matches;
    }

    static auto scanSignatureExpectOneResult(const SignatureAOI& signature) -> uintptr_t {
        auto results = scanSignature(signature);
        if (results.size() != 1) {
            Log::log("Signature Scan insuccessful");
            throw std::runtime_error("Signature Scan insuccessful");
        }
        return results.front();
    }
private:
    template<typename Scanner>
    static auto _scan_parallel(const Scanner& scanner,
                               const SignatureAOI& signatureAOI,
                               size_t amountThreads = std::thread::hardware_concurrency()) -> std::vector<uintptr_t> {
        auto executableAndReadable = [](const auto& range) {
            // skip segments that aren't code or are not readable
            return range.isReadable() && range.isExecutable();
        };
        auto librarySegments = MemoryUtils::lib_segment_ranges(signatureAOI.libName, executableAndReadable);

        size_t patternLength = signatureAOI.signature.pattern().length();
        auto thread_work_chunks = _get_thread_work_chunks(librarySegments, patternLength, amountThreads);

        std::vector<std::thread> threads;
        threads.reserve(amountThreads);
        std::vector<std::vector<uintptr_t>> allThreadMatches(amountThreads);
        for (size_t threadIdx = 0; threadIdx < amountThreads; ++threadIdx) {
            threads.emplace_back([threadIdx, &scanner, &thread_work_chunks, &allThreadMatches] {
                _thread_work(scanner, thread_work_chunks[threadIdx], allThreadMatches[threadIdx]);
            });
        }
        for (auto &thread : threads) {
            thread.join();
        }

        std::vector<uintptr_t> matches;
        for (auto& threadMatches : allThreadMatches) {
            for (uintptr_t match : threadMatches) {
                matches.push_back(match);
            }
        }

        return matches;
    }

    // split memory into evenly sized chunks
    static auto _get_thread_work_chunks(const std::vector<MemoryUtils::MemoryRange>& segments,
                                        size_t patternLength,
                                        size_t amount_threads) -> std::vector<std::vector<std::string_view>> {
        std::vector<std::vector<std::string_view>> per_thread_haystacks(amount_threads);
        int candidates_per_thread = _get_amount_candidates(segments, patternLength) / amount_threads;

        int segment_candidates_taken_by_last_thread = 0;
        for (size_t threadIdx = 0, segmentIdx = 0; threadIdx < amount_threads; ++threadIdx) {
            auto& work_for_this_thread = per_thread_haystacks[threadIdx];

            int candidates_left_for_thread = candidates_per_thread;
            for (; segmentIdx < segments.size(); ++segmentIdx) {
                auto& segment = segments[segmentIdx];
                bool isLastThread = threadIdx == amount_threads - 1;

                int segmentCandidates = static_cast<int>(segment.memoryRange.size() - (patternLength - 1));
                int candidates_left_in_segment = segmentCandidates - segment_candidates_taken_by_last_thread;

                int candidates_this_workload;
                int begin_offset = segment_candidates_taken_by_last_thread;
                if (isLastThread || candidates_left_in_segment <= candidates_left_for_thread) {
                    // take what's left
                    candidates_this_workload = candidates_left_in_segment;
                    segment_candidates_taken_by_last_thread = 0;
                }
                else { // candidates_left_in_segment > candidates_left_for_thread
                    candidates_this_workload = candidates_left_for_thread;
                    segment_candidates_taken_by_last_thread += candidates_left_for_thread;
                }

                candidates_left_for_thread -= candidates_this_workload;
                work_for_this_thread.emplace_back(segment.memoryRange.data() + begin_offset, candidates_this_workload + (patternLength - 1));

                if (!isLastThread && candidates_left_for_thread == 0) {
                    break; // move to next thread
                }
            }
        }
        return per_thread_haystacks;
    }

    static auto _get_amount_candidates(const std::vector<MemoryUtils::MemoryRange>& segments, size_t patternLength) -> int {
        int candidates = 0;
        for (auto& segment : segments) {
            int segmentSize = static_cast<int>(segment.memoryRange.size());
            auto segmentCandidates = segmentSize - (static_cast<int>(patternLength) - 1);
            if (segmentCandidates > 0) {
                candidates += segmentCandidates;
            }
        }
        return candidates;
    }

    template<typename Scanner>
    static auto _thread_work(const Scanner& scanner,
                             const std::vector<std::string_view> &haystacks,
                             std::vector<uintptr_t>& matches) {
        for (auto& haystack : haystacks) {
            scanner.scanSegment(haystack, matches);
        }
    }
};