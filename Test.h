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

#ifndef _TEST_H_
#define _TEST_H_

#include <string>

#include <vamp-hostsdk/Plugin.h>

class Test
{
public:
    virtual ~Test();

    enum Option {
        NoOption           = 0x0,
        NonDeterministic   = 0x1,
        Verbose            = 0x2,
        SingleTest         = 0x4
    };
    typedef int Options;
    
    class Result {

    public:
        enum Code { Success, Note, Warning, Error };

        Result() : m_code(Success) { }
        Result(Code c, std::string m) : m_code(c), m_message(m) { }

        Code code() const { return m_code; }
        std::string message() const { return m_message; }
        
    protected:
        Code m_code;
        std::string m_message;
    };

    static Result success() { return Result(Result::Success, ""); }
    static Result note(std::string m) { return Result(Result::Note, m); }
    static Result warning(std::string m) { return Result(Result::Warning, m); }
    static Result error(std::string m) { return Result(Result::Error, m); }

    typedef std::vector<Result> Results;

    class FailedToLoadPlugin { };

    // may throw FailedToLoadPlugin
    virtual Results test(std::string key, Options) = 0;

protected:
    Test();

    // may throw FailedToLoadPlugin
    Vamp::Plugin *load(std::string key, float rate = 44100);

    float **createBlock(size_t channels, size_t blocksize);
    void destroyBlock(float **blocks, size_t channels);

    float **createTestAudio(size_t channels, size_t blocksize, size_t blocks);
    void destroyTestAudio(float **audio, size_t channels);

    // use plugin's preferred step/block size, return them:
    bool initDefaults(Vamp::Plugin *, size_t &channels,
                      size_t &step, size_t &block, Results &r);

    // use the given step/block size and an adapter:
    bool initAdapted(Vamp::Plugin *, size_t &channels,
                     size_t step, size_t block, Results &r);

    void appendFeatures(Vamp::Plugin::FeatureSet &a,
                        const Vamp::Plugin::FeatureSet &b);

    bool allFeaturesValid(const Vamp::Plugin::FeatureSet &); // i.e. no NaN/inf

    bool containsTimestamps(const Vamp::Plugin::FeatureSet &);

    void dumpFeature(const Vamp::Plugin::Feature &, bool showValues);
    void dump(const Vamp::Plugin::FeatureSet &, bool showValues = true);
    void dumpTwo(const Result &r,
                 const Vamp::Plugin::FeatureSet &,
                 const Vamp::Plugin::FeatureSet &);
    void dumpDiff(const Result &r,
                  const Vamp::Plugin::FeatureSet &,
                  const Vamp::Plugin::FeatureSet &);
};

extern bool operator==(const Vamp::Plugin::FeatureSet &a,
                       const Vamp::Plugin::FeatureSet &b);
extern bool operator==(const Vamp::Plugin::FeatureList &a,
                       const Vamp::Plugin::FeatureList &b);
extern bool operator==(const Vamp::Plugin::Feature &a,
                       const Vamp::Plugin::Feature &b);

#endif

