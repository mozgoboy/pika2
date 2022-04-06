//
// This file is part of the pika parser reference implementation:
//
//     https://github.com/lukehutch/pikaparser
//
// The pika parsing algorithm is described in the following paper: 
//
//     Pika parsing: reformulating packrat parsing as a dynamic programming algorithm solves the left recursion
//     and error recovery problems. Luke A. D. Hutchison, May 2020.
//     https://arxiv.org/abs/2005.06444
//
// This software is provided under the MIT license:
//
// Copyright 2020 Luke A. D. Hutchison
//  
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
#pragma once
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <map>

using namespace std;
/** Grammar utils. */
class IntervalUnion {
private:
    map<int, int> nonOverlappingRanges;

    /** Add a range to a union of ranges. */
public: 
    void addRange(int startPos, int endPos) {
        if (endPos < startPos) {
            cout << "endPos < startPos";
            abort();
        }
        // Try merging new range with floor entry in TreeMap
        auto floorEntry = nonOverlappingRanges.upper_bound(startPos);
        int floorEntryStart = floorEntry == nonOverlappingRanges.end() ? 0 : prev(floorEntry)->first;
        int floorEntryEnd = floorEntry == nonOverlappingRanges.end() ? 0 : prev(floorEntry)->second;
        int newEntryRangeStart;
        int newEntryRangeEnd;
        if (floorEntryStart == 0 || floorEntryEnd < startPos) {
            // There is no startFloorEntry, or startFloorEntry ends before startPos -- add a new entry
            newEntryRangeStart = startPos;
            newEntryRangeEnd = endPos;
        }
        else {
            // startFloorEntry overlaps with range -- extend startFloorEntry
            newEntryRangeStart = floorEntryStart;
            newEntryRangeEnd = max(floorEntryEnd, endPos);
        }

        // Try merging new range with the following entry in TreeMap
        auto higherEntry = next(nonOverlappingRanges.lower_bound(newEntryRangeStart));
        int higherEntryStart = higherEntry == nonOverlappingRanges.end() ? 0 : higherEntry->first;
        int higherEntryEnd = higherEntry == nonOverlappingRanges.end() ? 0 : higherEntry->second;
        if (higherEntryStart != 0 && higherEntryStart <= newEntryRangeEnd) {
            // Expanded-range entry overlaps with the following entry -- collapse them into one
            nonOverlappingRanges.erase(higherEntryStart);
            int expandedRangeEnd = max(newEntryRangeEnd, higherEntryEnd);
            nonOverlappingRanges[newEntryRangeStart] = expandedRangeEnd;
        }
        else {
            // There's no overlap, just add the new entry (may overwrite the earlier entry for the range start)
            nonOverlappingRanges[newEntryRangeStart] = newEntryRangeEnd;
        }
    }

    /** Get the inverse of the intervals in this set within [StartPos, endPos). */
    IntervalUnion* invert(int startPos, int endPos) {
        IntervalUnion* invertedIntervalSet = new IntervalUnion();

        int prevEndPos = startPos;
        if (!nonOverlappingRanges.empty()) {
            for (auto ent : nonOverlappingRanges) {
                int currStartPos = ent.first;
                if (currStartPos > endPos) {
                    break;
                }
                int currEndPos = ent.second;
                if (currStartPos > prevEndPos) {
                    // There's a gap of at least one position between adjacent ranges
                    invertedIntervalSet->addRange(prevEndPos, currStartPos);
                }
                prevEndPos = currEndPos;
            }
            auto lastEnt = prev(nonOverlappingRanges.end());
            int lastEntEndPos = lastEnt->second;
            if (lastEntEndPos < endPos) {
                // Final range: there is at least one position before endPos
                invertedIntervalSet->addRange(lastEntEndPos, endPos);
            }
        }
        else {
            invertedIntervalSet->addRange(startPos, endPos);
        }
        return invertedIntervalSet;
    }

    /** Return true if the specified range overlaps with any range in this interval union. */
    bool rangeOverlaps(int startPos, int endPos) {
        // Range overlap test: https://stackoverflow.com/a/25369187/3950982
        // (Need to repeat for both floor entry and ceiling entry)
        auto floorEntry = nonOverlappingRanges.upper_bound(startPos);
        if (floorEntry != nonOverlappingRanges.end()) {
            int floorEntryStart = prev(floorEntry)->first;
            int floorEntryEnd = prev(floorEntry)->second;
            if (max(endPos, floorEntryEnd) - min(startPos, floorEntryStart) < (endPos - startPos)
                + (floorEntryEnd - floorEntryStart)) {
                return true;
            }
        }
        auto ceilEntry = nonOverlappingRanges.lower_bound(startPos);
        if (ceilEntry != nonOverlappingRanges.end()) {
            int ceilEntryStart = ceilEntry->first;
            int ceilEntryEnd = ceilEntry->second;
            if (max(endPos, ceilEntryEnd) - min(startPos, ceilEntryStart) < (endPos - startPos)
                + (ceilEntryEnd - ceilEntryStart)) {
                return true;
            }
        }
        return false;
    }

    /** Return all the nonoverlapping ranges in this interval union. */
    map<int, int> getNonOverlappingRanges() {
        return nonOverlappingRanges;
    }
};
