/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp Plugin Fuzz Tester
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

#include "Test.h"

#include <vamp-hostsdk/PluginLoader.h>

using namespace Vamp;
using namespace Vamp::HostExt;

#include <cmath>

Test::Test() { }
Test::~Test() { }

Plugin *
Test::load(std::string key, float rate)
{
    return PluginLoader::getInstance()->loadPlugin
        (key, rate, PluginLoader::ADAPT_ALL);
}

float **
Test::createBlock(size_t channels, size_t blocksize)
{
    float **b = new float *[channels];
    for (size_t c = 0; c < channels; ++c) {
        b[c] = new float[blocksize];
    }
    return b;
}

void
Test::destroyBlock(float **blocks, size_t channels)
{
    for (size_t c = 0; c < channels; ++c) {
        delete[] blocks[c];
    }
    delete[] blocks;
}

bool
Test::initDefaults(Plugin *p, size_t &channels, size_t &step, size_t &block,
                   Results &r)
{
    channels = p->getMinChannelCount();
    block = p->getPreferredBlockSize();
    step = p->getPreferredStepSize();
    if (block == 0) block = 1024;
    if (step == 0) {
        if (p->getInputDomain() == Plugin::FrequencyDomain) step = block/2;
        else step = block;
    }
    if (!p->initialise(channels, step, block)) {
        r.push_back(error("initialisation with default values failed"));
        return false;
    }
    return true;
}

void
Test::appendFeatures(Plugin::FeatureSet &a, const Plugin::FeatureSet &b)
{
    for (Plugin::FeatureSet::const_iterator i = b.begin(); i != b.end(); ++i) {
        int output = i->first;
        const Plugin::FeatureList &fl = i->second;
        Plugin::FeatureList &target = a[output];
        for (Plugin::FeatureList::const_iterator j = fl.begin(); j != fl.end(); ++j) {
            target.push_back(*j);
        }
    }
}

bool
Test::allFeaturesValid(const Plugin::FeatureSet &b)
{
    for (Plugin::FeatureSet::const_iterator i = b.begin(); i != b.end(); ++i) {
        for (int j = 0; j < (int)i->second.size(); ++j) {
            if (i->second[j].values.empty()) continue;
            for (int k = 0; k < (int)i->second[j].values.size(); ++k) {
                if (isnan(i->second[j].values[k]) ||
                    isinf(i->second[j].values[k])) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool
operator==(const Plugin::FeatureSet &a, const Plugin::FeatureSet &b)
{
    if (a.size() != b.size()) return false;
    for (Plugin::FeatureSet::const_iterator ai = a.begin();
         ai != a.end(); ++ai) {
        int output = ai->first;
        Plugin::FeatureSet::const_iterator bi = b.find(output);
        if (bi == b.end()) return false;
        if (!(ai->second == bi->second)) return false;
    }
    return true;
}

bool
operator==(const Plugin::FeatureList &a, const Plugin::FeatureList &b)
{
    if (a.size() != b.size()) return false;
    for (int i = 0; i < (int)a.size(); ++i) {
        if (!(a[i] == b[i])) return false;
    }
    return true;
}

bool
operator==(const Plugin::Feature &a, const Plugin::Feature &b)
{
    if (a.hasTimestamp != b.hasTimestamp) return false;
    if (a.hasTimestamp && (a.timestamp != b.timestamp)) return false;
    if (a.hasDuration != b.hasDuration) return false;
    if (a.hasDuration && (a.duration != b.duration)) return false;
    if (a.values != b.values) return false;
    if (a.label != b.label) return false;
    return true;
}

