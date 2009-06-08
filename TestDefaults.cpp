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

#include "TestDefaults.h"

#include <vamp-hostsdk/Plugin.h>
using namespace Vamp;

#include <memory>
using namespace std;

#include <cmath>
#include <time.h>

Tester::TestRegistrar<TestDefaultProgram>
TestDefaultProgram::m_registrar("E1 Inconsistent default program");

Tester::TestRegistrar<TestDefaultParameters>
TestDefaultParameters::m_registrar("E2 Inconsistent default parameters");

static const size_t _step = 1000;

Test::Results
TestDefaultProgram::test(string key, Options options)
{
    Plugin::FeatureSet f[2];
    int rate = 44100;
    Results r;
    float **data = 0;
    size_t channels = 0;
    size_t count = 100;

    for (int run = 0; run < 2; ++run) {
        auto_ptr<Plugin> p(load(key, rate));
	if (p->getPrograms().empty()) return r;
	if (run == 1) {
            p->selectProgram(p->getCurrentProgram());
        }
        if (!initAdapted(p.get(), channels, _step, _step, r)) return r;
        if (!data) data = createTestAudio(channels, _step, count);
        for (size_t i = 0; i < count; ++i) {
            float *ptr[channels];
            size_t idx = i * _step;
            for (size_t c = 0; c < channels; ++c) ptr[c] = data[c] + idx;
            RealTime timestamp = RealTime::frame2RealTime(idx, rate);
            Plugin::FeatureSet fs = p->process(ptr, timestamp);
            appendFeatures(f[run], fs);
        }
        Plugin::FeatureSet fs = p->getRemainingFeatures();
        appendFeatures(f[run], fs);
    }
    if (data) destroyTestAudio(data, channels);

    if (!(f[0] == f[1])) {
        string message = "Explicitly setting current program to its supposed current value changes the results";
        Result res;
        if (options & NonDeterministic) res = note(message);
        else res = error(message);
        if (options & Verbose) dump(res, f[0], f[1]);
        r.push_back(res);
    } else {
        r.push_back(success());
    }

    return r;
}

Test::Results
TestDefaultParameters::test(string key, Options options)
{
    Plugin::FeatureSet f[2];
    int rate = 44100;
    Results r;
    float **data = 0;
    size_t channels = 0;
    size_t count = 100;

    for (int run = 0; run < 2; ++run) {
        auto_ptr<Plugin> p(load(key, rate));
	if (p->getParameterDescriptors().empty()) return r;
	if (run == 1) {
            Plugin::ParameterList pl = p->getParameterDescriptors();
            for (int i = 0; i < (int)pl.size(); ++i) {
                if (p->getParameter(pl[i].identifier) != pl[i].defaultValue) {
                    if (options & Verbose) {
                        cout << "Parameter: " << pl[i].identifier << endl;
                        cout << "Expected: " << pl[i].defaultValue << endl;
                        cout << "Actual: " << p->getParameter(pl[i].identifier) << endl;
                    }
                    r.push_back(error("Not all parameters have their default values when queried directly after construction"));
                }
                p->setParameter(pl[i].identifier, pl[i].defaultValue);
            }
        }
        if (!initAdapted(p.get(), channels, _step, _step, r)) return r;
        if (!data) data = createTestAudio(channels, _step, count);
        for (size_t i = 0; i < count; ++i) {
            float *ptr[channels];
            size_t idx = i * _step;
            for (size_t c = 0; c < channels; ++c) ptr[c] = data[c] + idx;
            RealTime timestamp = RealTime::frame2RealTime(idx, rate);
            Plugin::FeatureSet fs = p->process(ptr, timestamp);
            appendFeatures(f[run], fs);
        }
        Plugin::FeatureSet fs = p->getRemainingFeatures();
        appendFeatures(f[run], fs);
    }
    if (data) destroyTestAudio(data, channels);

    if (!(f[0] == f[1])) {
        string message = "Explicitly setting parameters to their supposed default values changes the results";
        Result res;
        if (options & NonDeterministic) res = note(message);
        else res = error(message);
        if (options & Verbose) dump(res, f[0], f[1]);
        r.push_back(res);
    } else {
        r.push_back(success());
    }

    return r;
}
