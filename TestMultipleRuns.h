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

#ifndef _TEST_MULTIPLE_RUNS_H_
#define _TEST_MULTIPLE_RUNS_H_

#include "Test.h"
#include "Tester.h"

class TestDistinctRuns : public Test
{
public:
    TestDistinctRuns() : Test() { }
    Results test(std::string key, Options options);
    
protected:
    static Tester::TestRegistrar<TestDistinctRuns> m_registrar;
};

class TestReset : public Test
{
public:
    TestReset() : Test() { }
    Results test(std::string key, Options options);
    
protected:
    static Tester::TestRegistrar<TestReset> m_registrar;
};

class TestInterleavedRuns : public Test
{
public:
    TestInterleavedRuns() : Test() { }
    Results test(std::string key, Options options);
    
protected:
    static Tester::TestRegistrar<TestInterleavedRuns> m_registrar;
};

class TestDifferentStartTimes : public Test
{
public:
    TestDifferentStartTimes() : Test() { }
    Results test(std::string key, Options options);
    
protected:
    static Tester::TestRegistrar<TestDifferentStartTimes> m_registrar;
};

#endif
