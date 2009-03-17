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

#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginLoader.h>

#include <iostream>

#include <cstdlib>

#include "Tester.h"

using namespace std;

void usage(const char *name)
{
    cerr << "\n"
         << name << ": A Vamp plugin host that tests plugins for some likely errors.\n"
        "Chris Cannam, Centre for Digital Music, Queen Mary, University of London.\n"
        "Copyright 2009 QMUL.\n"
        "Freely redistributable; published under a BSD-style license.\n\n"
        "Usage:\n"
        "  " << name << " [<pluginbasename>:<plugin>]\n\n"
        "Example:\n"
        "  " << name << " vamp-example-plugins:amplitudefollower\n\n"
        "With an argument, tests one plugin; without, tests all plugins in Vamp path.\n"
        "If you have access to a runtime memory checker, you may find it especially\n"
        "helpful to run this tester under it and watch for errors thus provoked.\n"
         << endl;
    exit(2);
}

int main(int argc, char **argv)
{
    char *scooter = argv[0];
    char *name = 0;
    while (scooter && *scooter) {
        if (*scooter == '/' || *scooter == '\\') name = ++scooter;
        else ++scooter;
    }
    if (!name || !*name) name = argv[0];
    
    if (argc > 2) usage(name);
    if (argc == 2 && argv[1][0] == '-') usage(name);

    cerr << name << ": Running..." << endl;

    if (argc == 1) {
        bool good = true;
        Vamp::HostExt::PluginLoader::PluginKeyList keys =
            Vamp::HostExt::PluginLoader::getInstance()->listPlugins();
        for (int i = 0; i < (int)keys.size(); ++i) {
            cout << "Testing plugin: " << keys[i] << endl;
            Tester tester(keys[i]);
            if (tester.test()) {
                cout << name << ": All tests succeeded for this plugin" << endl;
            } else {
                cout << name << ": Some tests failed for this plugin" << endl;
                good = false;
            }
            cout << endl;
        }
        if (good) {
            cout << name << ": All tests succeeded" << endl;
            return 0;
        } else {
            cout << name << ": Some tests failed" << endl;
            return 1;
        }   
    } else {
        string key = argv[1];
        Tester tester(key);
        if (tester.test()) {
            cout << name << ": All tests succeeded" << endl;
            return 0;
        } else {
            cout << name << ": Some tests failed" << endl;
            return 1;
        }
    }
}

