/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp Plugin Tester
    Chris Cannam, cannam@all-day-breakfast.com
    Centre for Digital Music, Queen Mary, University of London.
    Copyright 2009 QMUL.

    This program loads a Vamp plugin and tests its susceptibility to a
    number of common pitfalls, including handling of extremes of input
    data.  If you can think of any additional useful tests that are
    easily added, please send them to me.
  
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of the Centre for
    Digital Music; Queen Mary, University of London; and Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/

#include "TestMultipleRuns.h"

#include <vamp-hostsdk/Plugin.h>
using namespace Vamp;

#include <memory>
using namespace std;

#include <cmath>

Tester::TestRegistrar<TestDistinctRuns>
TestDistinctRuns::m_registrar("Consecutive runs with separate instances");

Tester::TestRegistrar<TestReset>
TestReset::m_registrar("Consecutive runs with a single instance using reset");

Tester::TestRegistrar<TestInterleavedRuns>
TestInterleavedRuns::m_registrar("Simultaneous interleaved runs in a single thread");

Test::Results
TestDistinctRuns::test(string key)
{
    Plugin::FeatureSet f[2];
    int rate = 44100;
    Results r;
    float **block = 0;
    size_t channels = 0, step = 0, blocksize = 0;

    for (int run = 0; run < 2; ++run) {
        auto_ptr<Plugin> p(load(key, rate));
        if (!initDefaults(p.get(), channels, step, blocksize, r)) return r;
        if (!block) block = createBlock(channels, blocksize);
        int idx = 0;
        for (int i = 0; i < 100; ++i) {
            for (size_t j = 0; j < blocksize; ++j) {
                for (size_t c = 0; c < channels; ++c) {
                    block[c][j] = sinf(float(idx) / 10.f);
                    if ((i == 20 || i == 80) && (j < 2)) {
                        block[c][j] = float(j) - 1.f;
                    }
                }
                ++idx;
            }
            RealTime timestamp = RealTime::frame2RealTime(idx, rate);
            Plugin::FeatureSet fs = p->process(block, timestamp);
            appendFeatures(f[run], fs);
        }
        Plugin::FeatureSet fs = p->getRemainingFeatures();
        appendFeatures(f[run], fs);
    }
    if (block) destroyBlock(block, channels);

    if (!(f[0] == f[1])) {
        r.push_back(warning("Consecutive runs with separate instances produce different results"));
    } else {
        r.push_back(success());
    }

    return r;
}

Test::Results
TestReset::test(string key)
{
    Plugin::FeatureSet f[2];
    int rate = 44100;
    Results r;
    float **block = 0;
    size_t channels = 0, step = 0, blocksize = 0;

    auto_ptr<Plugin> p(load(key, rate));
    for (int run = 0; run < 2; ++run) {
        if (run == 1) p->reset();
        if (!initDefaults(p.get(), channels, step, blocksize, r)) return r;
        if (!block) block = createBlock(channels, blocksize);
        int idx = 0;
        for (int i = 0; i < 100; ++i) {
            for (size_t j = 0; j < blocksize; ++j) {
                for (size_t c = 0; c < channels; ++c) {
                    block[c][j] = sinf(float(idx) / 10.f);
                    if ((i == 20 || i == 80) && (j < 2)) {
                        block[c][j] = float(j) - 1.f;
                    }
                }
                ++idx;
            }
            RealTime timestamp = RealTime::frame2RealTime(idx, rate);
            Plugin::FeatureSet fs = p->process(block, timestamp);
            appendFeatures(f[run], fs);
        }
        Plugin::FeatureSet fs = p->getRemainingFeatures();
        appendFeatures(f[run], fs);
    }
    if (block) destroyBlock(block, channels);

    if (!(f[0] == f[1])) {
        r.push_back(warning("Consecutive runs with the same instance (using reset) produce different results"));
    } else {
        r.push_back(success());
    }

    return r;
}

Test::Results
TestInterleavedRuns::test(string key)
{
    Plugin::FeatureSet f[2];
    int rate = 44100;
    Results r;
    float **block = 0;
    size_t channels = 0, step = 0, blocksize = 0;
    Plugin *p[2];
    for (int run = 0; run < 2; ++run) {
        p[run] = load(key, rate);
        if (!initDefaults(p[run], channels, step, blocksize, r)) {
            delete p[run];
            if (run > 0) delete p[0];
            return r;
        }
        if (!block) block = createBlock(channels, blocksize);
    }
    int idx = 0;
    for (int i = 0; i < 100; ++i) {
        for (size_t j = 0; j < blocksize; ++j) {
            for (size_t c = 0; c < channels; ++c) {
                block[c][j] = sinf(float(idx) / 10.f);
                if ((i == 20 || i == 80) && (j < 2)) {
                    block[c][j] = float(j) - 1.f;
                }
            }
            ++idx;
        }
        RealTime timestamp = RealTime::frame2RealTime(idx, rate);
        for (int run = 0; run < 2; ++run) {
            Plugin::FeatureSet fs = p[run]->process(block, timestamp);
            appendFeatures(f[run], fs);
        }
    }
    for (int run = 0; run < 2; ++run) {
        Plugin::FeatureSet fs = p[run]->getRemainingFeatures();
        appendFeatures(f[run], fs);
        delete p[run];
    }

    if (block) destroyBlock(block, channels);

    if (!(f[0] == f[1])) {
        r.push_back(warning("Consecutive runs with the same instance (using reset) produce different results"));
    } else {
        r.push_back(success());
    }

    return r;
}
