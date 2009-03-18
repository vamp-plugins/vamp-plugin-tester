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

#include "TestInputExtremes.h"

#include <vamp-hostsdk/Plugin.h>
using namespace Vamp;

#include <memory>
using namespace std;

#include <cstdlib>
#include <cmath>

Tester::TestRegistrar<TestNormalInput>
TestNormalInput::m_registrar("C1 Normal input");

Tester::TestRegistrar<TestNoInput>
TestNoInput::m_registrar("C2 Empty input");

Tester::TestRegistrar<TestShortInput>
TestShortInput::m_registrar("C3 Short input");

Tester::TestRegistrar<TestSilentInput>
TestSilentInput::m_registrar("C4 Absolutely silent input");

Tester::TestRegistrar<TestTooLoudInput>
TestTooLoudInput::m_registrar("C5 Input beyond expected +/-1 range");

Tester::TestRegistrar<TestRandomInput>
TestRandomInput::m_registrar("C6 Random input");

Test::Results
TestNormalInput::test(string key, Options options)
{
    Plugin::FeatureSet f;
    int rate = 44100;
    auto_ptr<Plugin> p(load(key, rate));
    Results r;
    size_t channels, step, blocksize;
    if (!initDefaults(p.get(), channels, step, blocksize, r)) return r;
    float **block = createBlock(channels, blocksize);
    int idx = 0;
    for (int i = 0; i < 200; ++i) {
        for (size_t j = 0; j < blocksize; ++j) {
            for (size_t c = 0; c < channels; ++c) {
                block[c][j] = sinf(float(idx) / 10.f);
            }
            ++idx;
        }
        RealTime timestamp = RealTime::frame2RealTime(idx, rate);
        Plugin::FeatureSet fs = p->process(block, timestamp);
        appendFeatures(f, fs);
    }
    destroyBlock(block, channels);
    Plugin::FeatureSet fs = p->getRemainingFeatures();
    appendFeatures(f, fs);
    if (allFeaturesValid(f)) {
        r.push_back(success());
    } else {
        r.push_back(warning("plugin returned one or more NaN/inf values"));
        if (options & Verbose) dump(f);
    }
    return r;
}

Test::Results
TestNoInput::test(string key, Options options)
{
    auto_ptr<Plugin> p(load(key));
    Results r;
    size_t channels, step, block;
    if (!initDefaults(p.get(), channels, step, block, r)) return r;
    Plugin::FeatureSet fs = p->getRemainingFeatures();
    if (allFeaturesValid(fs)) {
        r.push_back(success());
    } else {
        r.push_back(warning("plugin returned one or more NaN/inf values"));
    }
    return r;
}

Test::Results
TestShortInput::test(string key, Options options)
{
    Plugin::FeatureSet f;
    int rate = 44100;
    auto_ptr<Plugin> p(load(key, rate));
    Results r;
    size_t channels, step, blocksize;
    if (!initDefaults(p.get(), channels, step, blocksize, r)) return r;
    float **block = createBlock(channels, blocksize);
    int idx = 0;
    for (size_t j = 0; j < blocksize; ++j) {
        for (size_t c = 0; c < channels; ++c) {
            block[c][j] = sinf(float(idx) / 10.f);
        }
        ++idx;
    }
    Plugin::FeatureSet fs = p->process(block, RealTime::zeroTime);
    appendFeatures(f, fs);
    destroyBlock(block, channels);
    fs = p->getRemainingFeatures();
    appendFeatures(f, fs);
    if (allFeaturesValid(f)) {
        r.push_back(success());
    } else {
        r.push_back(warning("plugin returned one or more NaN/inf values"));
        if (options & Verbose) dump(f);
    }
    return r;
}

Test::Results
TestSilentInput::test(string key, Options options)
{
    Plugin::FeatureSet f;
    int rate = 44100;
    auto_ptr<Plugin> p(load(key, rate));
    Results r;
    size_t channels, step, blocksize;
    if (!initDefaults(p.get(), channels, step, blocksize, r)) return r;
    float **block = createBlock(channels, blocksize);
    for (size_t j = 0; j < blocksize; ++j) {
        for (size_t c = 0; c < channels; ++c) {
            block[c][j] = 0.f;
        }
    }
    for (int i = 0; i < 200; ++i) {
        RealTime timestamp = RealTime::frame2RealTime(i * blocksize, rate);
        Plugin::FeatureSet fs = p->process(block, timestamp);
        appendFeatures(f, fs);
    }
    destroyBlock(block, channels);
    Plugin::FeatureSet fs = p->getRemainingFeatures();
    appendFeatures(f, fs);
    if (allFeaturesValid(f)) {
        r.push_back(success());
    } else {
        r.push_back(warning("plugin returned one or more NaN/inf values"));
        if (options & Verbose) dump(f);
    }
    return r;
}

Test::Results
TestTooLoudInput::test(string key, Options options)
{
    Plugin::FeatureSet f;
    int rate = 44100;
    auto_ptr<Plugin> p(load(key, rate));
    Results r;
    size_t channels, step, blocksize;
    if (!initDefaults(p.get(), channels, step, blocksize, r)) return r;
    float **block = createBlock(channels, blocksize);
    int idx = 0;
    for (int i = 0; i < 200; ++i) {
        for (size_t j = 0; j < blocksize; ++j) {
            for (size_t c = 0; c < channels; ++c) {
                block[c][j] = 1000.f * sinf(float(idx) / 10.f);
            }
            ++idx;
        }
        RealTime timestamp = RealTime::frame2RealTime(idx, rate);
        Plugin::FeatureSet fs = p->process(block, timestamp);
        appendFeatures(f, fs);
    }
    destroyBlock(block, channels);
    Plugin::FeatureSet fs = p->getRemainingFeatures();
    appendFeatures(f, fs);
    if (allFeaturesValid(f)) {
        r.push_back(success());
    } else {
        r.push_back(warning("plugin returned one or more NaN/inf values"));
        if (options & Verbose) dump(f);
    }
    return r;
}

Test::Results
TestRandomInput::test(string key, Options options)
{
    Plugin::FeatureSet f;
    int rate = 44100;
    auto_ptr<Plugin> p(load(key, rate));
    Results r;
    size_t channels, step, blocksize;
    if (!initDefaults(p.get(), channels, step, blocksize, r)) return r;
    float **block = createBlock(channels, blocksize);
    int idx = 0;
    for (int i = 0; i < 100; ++i) {
        for (size_t j = 0; j < blocksize; ++j) {
            for (size_t c = 0; c < channels; ++c) {
                block[c][j] = float(drand48() * 2.0 - 1.0);
            }
            ++idx;
        }
        RealTime timestamp = RealTime::frame2RealTime(idx, rate);
        Plugin::FeatureSet fs = p->process(block, timestamp);
        appendFeatures(f, fs);
    }
    destroyBlock(block, channels);
    Plugin::FeatureSet fs = p->getRemainingFeatures();
    appendFeatures(f, fs);
    if (allFeaturesValid(f)) {
        r.push_back(success());
    } else {
        r.push_back(warning("plugin returned one or more NaN/inf values"));
        if (options & Verbose) dump(f);
    }
    return r;
}

