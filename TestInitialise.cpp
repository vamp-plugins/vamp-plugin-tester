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

#include "TestInitialise.h"

#include <vamp-hostsdk/Plugin.h>
#include <vamp-hostsdk/PluginLoader.h>
using namespace Vamp;
using namespace Vamp::HostExt;

#include <set>
#include <memory>
using namespace std;

#include <cmath>

#ifndef __GNUC__
#include <alloca.h>
#endif

Tester::TestRegistrar<TestSampleRates>
TestSampleRates::m_registrar("F1", "Different sample rates");

Tester::TestRegistrar<TestLengthyConstructor>
TestLengthyConstructor::m_registrar("F2", "Lengthy constructor");

static const size_t _step = 1000;

Test::Results
TestSampleRates::test(string key, Options options)
{
    int rates[] =
        { 11, 800, 10099, 11024, 44100, 48000, 96000, 192000, 201011, 1094091 };

    Results r;

    if (options & Verbose) {
        cout << "    ";
    }

    for (int i = 0; i < int(sizeof(rates)/sizeof(rates[0])); ++i) {
    
        int rate = rates[i];

        if (options & Verbose) {
            cout << "[" << rate << "Hz] " << flush;
        }

        auto_ptr<Plugin> p(load(key, rate));
        Plugin::FeatureSet f;
        float **data = 0;
        size_t channels = 0;
        size_t count = 100;

        Results subr;
        if (!initAdapted(p.get(), channels, _step, _step, subr)) {
            // This is not an error; the plugin can legitimately
            // refuse to initialise at weird settings and that's often
            // the most acceptable result
            if (!subr.empty()) {
                r.push_back(note(subr.begin()->message()));
            }
            continue;
        }

        data = createTestAudio(channels, _step, count);
        for (size_t i = 0; i < count; ++i) {
#ifdef __GNUC__
            float *ptr[channels];
#else
            float **ptr = (float **)alloca(channels * sizeof(float));
#endif
            size_t idx = i * _step;
            for (size_t c = 0; c < channels; ++c) ptr[c] = data[c] + idx;
            RealTime timestamp = RealTime::frame2RealTime(idx, rate);
            Plugin::FeatureSet fs = p->process(ptr, timestamp);
            appendFeatures(f, fs);
        }
        Plugin::FeatureSet fs = p->getRemainingFeatures();
        appendFeatures(f, fs);
        destroyTestAudio(data, channels);
    }

    if (options & Verbose) cout << endl;

    // We can't actually do anything meaningful with our results.
    // We're really just testing to see whether the plugin crashes.  I
    // wonder whether it's possible to do any better?  If not, we
    // should probably report our limitations

    return r;
}

Test::Results
TestLengthyConstructor::test(string key, Options)
{
    time_t t0 = time(0);
    auto_ptr<Plugin> p(load(key));
    time_t t1 = time(0);
    Results r;
    if (t1 - t0 > 1) r.push_back(warning("Constructor takes some time to run: work should be deferred to initialise?"));
    return r;
}

