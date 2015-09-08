/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp Plugin Fuzz Tester
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

#include "Test.h"

#include <vamp-hostsdk/PluginLoader.h>

using namespace Vamp;
using namespace Vamp::HostExt;

#include <math.h>

#ifdef __SUNPRO_CC
#include <ieeefp.h>
#define isinf(x) (!finite(x))
#endif

Test::Test() { }
Test::~Test() { }

using std::cerr;
using std::cout;
using std::endl;
using std::string;

Plugin *
Test::load(string key, float rate)
{
    Plugin *p = PluginLoader::getInstance()->loadPlugin
        (key, rate, PluginLoader::ADAPT_ALL);
    if (!p) throw FailedToLoadPlugin();
    return p;
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

float **
Test::createTestAudio(size_t channels, size_t blocksize, size_t blocks)
{
    float **b = new float *[channels];
    for (size_t c = 0; c < channels; ++c) {
        b[c] = new float[blocksize * blocks];
        for (int i = 0; i < int(blocksize * blocks); ++i) {
            b[c][i] = sinf(float(i) / 10.f);
            if (i == 5005 || i == 20002) {
                b[c][i-2] = 0;
                b[c][i-1] = -1;
                b[c][i] = 1;
            }
        }
    }
    return b;
}

void
Test::destroyTestAudio(float **b, size_t channels)
{
    for (size_t c = 0; c < channels; ++c) {
        delete[] b[c];
    }
    delete[] b;
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

bool
Test::initAdapted(Plugin *p, size_t &channels, size_t step, size_t block,
                  Results &r)
{
    channels = p->getMinChannelCount();
    if (!p->initialise(channels, step, block)) {
        r.push_back(error("initialisation failed"));
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
Test::containsTimestamps(const Plugin::FeatureSet &b)
{
    for (Plugin::FeatureSet::const_iterator i = b.begin(); i != b.end(); ++i) {
        for (int j = 0; j < (int)i->second.size(); ++j) {
            if (i->second[j].values.empty()) continue;
            for (int k = 0; k < (int)i->second[j].values.size(); ++k) {
                if (i->second[j].hasTimestamp) {
                    return true;
                }
            }
        }
    }
    return false;
}

void
Test::dumpFeature(const Plugin::Feature &f, bool showValues,
                  const Plugin::Feature *other)
{
    cout << "    Timestamp: " << (!f.hasTimestamp ? "(none)" : f.timestamp.toText()) << endl;
    cout << "    Duration: " << (!f.hasDuration ? "(none)" : f.duration.toText()) << endl;
    cout << "    Label: " << (f.label == "" ? "(none)" : f.label) << endl;
    if (showValues) {
        cout << "    Values (" << f.values.size() << "): " << (f.values.empty() ? "(none)" : "");
        int n = f.values.size();
        if (!other) {
            for (int j = 0; j < n; ++j) {
                cout << f.values[j] << " ";
            }
        } else {
            int samecount = 0;
            int diffcount = 0;
            for (int j = 0; j <= n; ++j) {
                if (j < n && f.values[j] == other->values[j]) {
                    ++samecount;
                } else {
                    if (samecount > 0) {
                        cout << "(" << samecount << " identical) ";
                    }
                    samecount = 0;
                    if (j < n) {
                        ++diffcount;
                        if (diffcount > 20 && j + 10 < n) {
                            cout << "(remaining " << n - j << " values elided)";
                            break;
                        } else {
                            cout << f.values[j] << " [diff "
                                 << f.values[j] - other->values[j] << "] ";
                        }
                    }
                }
            }
        }
        cout << endl;
    } else {
        cout << "    Values (" << f.values.size() << "): (elided)" << endl;
    }
}    

void
Test::dump(const Plugin::FeatureSet &fs, bool showValues)
{
    for (Plugin::FeatureSet::const_iterator fsi = fs.begin();
         fsi != fs.end(); ++fsi) {
        int output = fsi->first;
        cout << "Output " << output << ":" << endl;
        const Plugin::FeatureList &fl = fsi->second;
        for (int i = 0; i < (int)fl.size(); ++i) {
            cout << "  Feature " << i << ":" << endl;
            const Plugin::Feature &f = fl[i];
            dumpFeature(f, showValues);
        }
    }
}

void
Test::dumpTwo(const Result &r,
              const Plugin::FeatureSet &a,
              const Plugin::FeatureSet &b)
{
    std::cout << r.message() << std::endl;
    std::cout << "\nFirst result set:" << std::endl;
    dump(a, false);
    std::cout << "\nSecond result set:" << std::endl;
    dump(b, false);
    std::cout << std::endl;
}

void
Test::dumpDiff(const Result &r,
               const Plugin::FeatureSet &a,
               const Plugin::FeatureSet &b)
{
    cout << r.message() << endl;
    cout << "\nDifferences follow:" << endl;
    if (a.size() != b.size()) {
        cout << "*** First result set has features on " << a.size() 
                  << " output(s), second has features on " << b.size()
                  << endl;
        return;
    }
    Plugin::FeatureSet::const_iterator ai = a.begin();
    Plugin::FeatureSet::const_iterator bi = b.begin();
    while (ai != a.end()) {
        if (ai->first != bi->first) {
            cout << "\n*** Output number mismatch: first result set says "
                      << ai->first << " where second says " << bi->first
                      << endl;
        } else {
            cout << "\nOutput " << ai->first << ":" << endl;
            if (ai->second.size() != bi->second.size()) {
                cout << "*** First result set has " << ai->second.size()
                          << " feature(s) on this output, second has "
                          << bi->second.size() << endl;
            } else {
                int fno = 0;
                int diffcount = 0;
                Plugin::FeatureList::const_iterator afi = ai->second.begin();
                Plugin::FeatureList::const_iterator bfi = bi->second.begin();
                while (afi != ai->second.end()) {
                    if (!(*afi == *bfi)) {
                        if (diffcount == 0) {
                            bool differInValues =
                                (afi->values.size() == bfi->values.size() &&
                                 afi->values != bfi->values);
                            if (afi->hasTimestamp != bfi->hasTimestamp) {
                                cout << "*** Feature " << fno << " differs in presence of timestamp (" << afi->hasTimestamp << " vs " << bfi->hasTimestamp << ")" << endl;
                            }
                            if (afi->hasTimestamp && (afi->timestamp != bfi->timestamp)) {
                                cout << "*** Feature " << fno << " differs in timestamp (" << afi->timestamp << " vs " << bfi->timestamp << " )" << endl;
                            }
                            if (afi->hasDuration != bfi->hasDuration) {
                                cout << "*** Feature " << fno << " differs in presence of duration (" << afi->hasDuration << " vs " << bfi->hasDuration << ")" << endl;
                            }
                            if (afi->hasDuration && (afi->duration != bfi->duration)) {
                                cout << "*** Feature " << fno << " differs in duration (" << afi->duration << " vs " << bfi->duration << " )" << endl;
                            }
                            if (afi->label != bfi->label) {
                                cout << "*** Feature " << fno << " differs in label" << endl;
                            }
                            if (afi->values.size() != bfi->values.size()) {
                                cout << "*** Feature " << fno << " differs in number of values (" << afi->values.size() << " vs " << bfi->values.size() << ")" << endl;
                            }
                            if (differInValues) {
                                cout << "*** Feature " << fno << " differs in values" << endl;
                            }
                            cout << "  First output:" << endl;
                            dumpFeature(*afi, differInValues);
                            cout << "  Second output:" << endl;
                            dumpFeature(*bfi, differInValues, &(*afi));
                        }
                        ++diffcount;
                    }
                    ++fno;
                    ++afi;
                    ++bfi;
                }
                if (diffcount > 1) {
                    cout << diffcount-1 << " subsequent differing feature(s) elided" << endl;
                }
            }
        }                
        ++ai;
        ++bi;
    }
    cout << endl;
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

