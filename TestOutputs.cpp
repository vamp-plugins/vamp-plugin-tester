/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp Plugin Tester
    Chris Cannam, cannam@all-day-breakfast.com
    Centre for Digital Music, Queen Mary, University of London.
    Copyright 2009-2014 QMUL.

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

#include "TestOutputs.h"

#include <vamp-hostsdk/Plugin.h>
#include <vamp-hostsdk/PluginLoader.h>
using namespace Vamp;
using namespace Vamp::HostExt;

#include <set>
#include <memory>
using namespace std;

#include <cmath>

Tester::TestRegistrar<TestOutputNumbers>
TestOutputNumbers::m_registrar("B1", "Output number mismatching");

Tester::TestRegistrar<TestTimestamps>
TestTimestamps::m_registrar("B2", "Invalid or dubious timestamp usage");

static const size_t _step = 1000;

Test::Results
TestOutputNumbers::test(string key, Options options)
{
    int rate = 44100;
    unique_ptr<Plugin> p(load(key, rate));
    Plugin::FeatureSet f;
    Results r;
    float **data = 0;
    size_t channels = 0;
    size_t count = 100;

    if (!initAdapted(p.get(), channels, _step, _step, r)) return r;
    if (!data) data = createTestAudio(channels, _step, count);
    for (size_t i = 0; i < count; ++i) {
        float **ptr = new float *[channels];
        size_t idx = i * _step;
        for (size_t c = 0; c < channels; ++c) ptr[c] = data[c] + idx;
        RealTime timestamp = RealTime::frame2RealTime(idx, rate);
        Plugin::FeatureSet fs = p->process(ptr, timestamp);
        delete[] ptr;
        appendFeatures(f, fs);
    }
    Plugin::FeatureSet fs = p->getRemainingFeatures();
    appendFeatures(f, fs);
    if (data) destroyTestAudio(data, channels);

    std::set<int> used;
    Plugin::OutputList outputs = p->getOutputDescriptors();
    for (Plugin::FeatureSet::const_iterator i = f.begin();
         i != f.end(); ++i) {
        int o = i->first;
        used.insert(o);
        if (o < 0 || o >= (int)outputs.size()) {
            r.push_back(error("Data returned on nonexistent output"));
        }
    }
    for (int o = 0; o < (int)outputs.size(); ++o) {
        if (used.find(o) == used.end()) {
            r.push_back(note("No results returned for output \"" + outputs[o].identifier + "\"")); 
        }
    }
                
    if (!r.empty() && (options & Verbose)) dump(f);
    return r;
}

Test::Results
TestTimestamps::test(string key, Options options)
{
    int rate = 44100;

    // we want to be sure that a buffer size adapter is not used:
    unique_ptr<Plugin> p(PluginLoader::getInstance()->loadPlugin
                       (key, rate, PluginLoader::ADAPT_ALL_SAFE));

    Results r;
    Plugin::FeatureSet f;
    float **data = 0;
    size_t channels = 0;
    size_t step = 0, block = 0;
    size_t count = 100;

    if (!initDefaults(p.get(), channels, step, block, r)) return r;

    Plugin::OutputList outputs = p->getOutputDescriptors();
    for (int i = 0; i < (int)outputs.size(); ++i) {
        if (outputs[i].sampleType == Plugin::OutputDescriptor::FixedSampleRate &&
            outputs[i].sampleRate == 0.f) {
            r.push_back(error("Plugin output \"" + outputs[i].identifier +
                              "\" has FixedSampleRate but gives sample rate as 0"));
        }
    }

    if (!data) data = createTestAudio(channels, block, count);
    for (size_t i = 0; i < count; ++i) {
        float **ptr = new float *[channels];
        size_t idx = i * step;
        for (size_t c = 0; c < channels; ++c) ptr[c] = data[c] + idx;
        RealTime timestamp = RealTime::frame2RealTime(idx, rate);
        Plugin::FeatureSet fs = p->process(ptr, timestamp);
        delete[] ptr;
        appendFeatures(f, fs);
    }
    Plugin::FeatureSet fs = p->getRemainingFeatures();
    appendFeatures(f, fs);
    if (data) destroyTestAudio(data, channels);

    for (Plugin::FeatureSet::const_iterator i = f.begin();
         i != f.end(); ++i) {
        const Plugin::OutputDescriptor &o = outputs[i->first];
        const Plugin::FeatureList &fl = i->second;
        for (int j = 0; j < (int)fl.size(); ++j) {
            const Plugin::Feature &fe = fl[j];
            switch (o.sampleType) {
            case Plugin::OutputDescriptor::OneSamplePerStep:
                if (fe.hasTimestamp) {
                    r.push_back(note("Plugin returns features with timestamps on OneSamplePerStep output \"" + o.identifier + "\""));
                }
                if (fe.hasDuration) {
                    r.push_back(note("Plugin returns features with durations on OneSamplePerStep output \"" + o.identifier + "\""));
                }
                break;
            case Plugin::OutputDescriptor::FixedSampleRate:
                break;
            case Plugin::OutputDescriptor::VariableSampleRate:
                if (!fe.hasTimestamp) {
                    r.push_back(error("Plugin returns features with no timestamps on VariableSampleRate output \"" + o.identifier + "\""));
                }
                break;
            }
        }
    }

    if (!r.empty() && (options & Verbose)) dump(f);
    return r;
}
