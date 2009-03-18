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

#include "TestStaticData.h"

#include <vamp-hostsdk/Plugin.h>
using namespace Vamp;

#include <memory>
using namespace std;

#include <cmath>

Tester::TestRegistrar<TestIdentifiers>
TestIdentifiers::m_registrar("A1 Invalid identifiers");

Tester::TestRegistrar<TestEmptyFields>
TestEmptyFields::m_registrar("A2 Empty metadata fields");

Tester::TestRegistrar<TestValueRanges>
TestValueRanges::m_registrar("A3 Inappropriate value extents");

Test::Results
TestIdentifiers::test(string key, Options options)
{
    auto_ptr<Plugin> p(load(key));
    
    Results r;
    r.push_back(testIdentifier(p->getIdentifier(), "plugin identifier"));

    Plugin::ParameterList params = p->getParameterDescriptors();
    for (int i = 0; i < (int)params.size(); ++i) {
        r.push_back(testIdentifier(params[i].identifier, "parameter identifier"));
    }

    Plugin::OutputList outputs = p->getOutputDescriptors();
    for (int i = 0; i < (int)outputs.size(); ++i) {
        r.push_back(testIdentifier(outputs[i].identifier, "output identifier"));
    }

    return r;
}

Test::Result
TestIdentifiers::testIdentifier(string identifier, string desc)
{
    for (int i = 0; i < (int)identifier.length(); ++i) {
        char c = identifier[i];
        if (c >= 'a' && c <= 'z') continue;
        if (c >= 'A' && c <= 'Z') continue;
        if (c >= '0' && c <= '9') continue;
        if (c == '_' || c == '-') continue;
        return error
            (desc + " \"" + identifier +
             "\" contains invalid character(s); permitted are: [a-zA-Z0-9_-]");
    }
    return success();
}

Test::Results
TestEmptyFields::test(string key, Options options)
{
    auto_ptr<Plugin> p(load(key));

    Results r;

    r.push_back(testMandatory(p->getName(), "plugin name"));
    r.push_back(testRecommended(p->getDescription(), "plugin description"));
    r.push_back(testRecommended(p->getMaker(), "plugin maker"));
    r.push_back(testRecommended(p->getCopyright(), "plugin copyright"));
    
    Plugin::ParameterList params = p->getParameterDescriptors();
    for (int i = 0; i < (int)params.size(); ++i) {
        r.push_back(testMandatory
                    (params[i].name,
                     "plugin parameter \"" + params[i].identifier + "\" name"));
        r.push_back(testRecommended
                    (params[i].description,
                     "plugin parameter \"" + params[i].identifier + "\" description"));
    }
    
    Plugin::OutputList outputs = p->getOutputDescriptors();
    for (int i = 0; i < (int)outputs.size(); ++i) {
        r.push_back(testMandatory
                    (outputs[i].name,
                     "plugin output \"" + outputs[i].identifier + "\" name"));
        r.push_back(testRecommended
                    (outputs[i].description,
                     "plugin output \"" + outputs[i].identifier + "\" description"));
    }

    return r;
}

Test::Result
TestEmptyFields::testMandatory(string text, string desc)
{
    if (text == "") {
        return error(desc + " is empty");
    }
    return success();
}

Test::Result
TestEmptyFields::testRecommended(string text, string desc)
{
    if (text == "") {
        return warning(desc + " is empty");
    }
    return success();
}

Test::Results
TestValueRanges::test(string key, Options options)
{
    auto_ptr<Plugin> p(load(key));

    Results r;

    Plugin::ParameterList params = p->getParameterDescriptors();
    for (int i = 0; i < (int)params.size(); ++i) {
        Plugin::ParameterDescriptor &pd(params[i]);
        string pfx("plugin parameter \"" + pd.identifier + "\"");
        float min = pd.minValue;
        float max = pd.maxValue;
        float deft = pd.defaultValue;
        if (max <= min) {
            r.push_back(error(pfx + " maxValue <= minValue"));
        }
        if (deft < min || deft > max) {
            r.push_back(error(pfx + " defaultValue out of range"));
        }
        if (pd.isQuantized) {
            if (pd.quantizeStep == 0.f) {
                r.push_back(error(pfx + " is quantized, but quantize step is zero"));
            } else {

                float epsilon = 0.00001f;
                int qty = int((max - min) / pd.quantizeStep + 0.5);
                float target = min + pd.quantizeStep * qty;
                if (fabsf(max - target) > epsilon) {
                    r.push_back(warning(pfx + " value range is not a multiple of quantize step"));
                }

                if (!pd.valueNames.empty()) {
                    if ((int)pd.valueNames.size() < qty+1) {
                        r.push_back(warning(pfx + " has fewer value names than quantize steps"));
                    } else if ((int)pd.valueNames.size() > qty+1) {
                        r.push_back(warning(pfx + " has more value names than quantize steps"));
                    }
                }

                qty = int((deft - min) / pd.quantizeStep + 0.5);
                target = min + pd.quantizeStep * qty;
                if (fabsf(deft - target) > epsilon) {
                    r.push_back(warning(pfx + " default value is not a multiple of quantize step beyond minimum"));
                }
            }
        }
    }

    return r;
}


