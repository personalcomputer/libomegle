libomegle Readme
================

libomegle is a little C++ (or python with provided bindings) library for interfacing with the 'Omegle' <http://omegle.com/> random-pairing instant messaging service.

See the demos, demos/omegle-cli-client.cpp and demos/basic-chatbot.cpp

An example chatlog courtesy of basic-chatbot:

    (connected)
    Stranger: hi
    (Bot): Tell me more
    Stranger: wat???
    (Bot): I see
    Stranger: wat u see???
    (Bot): I see
    Stranger: ooooo
    (Bot): Are you sure?
    Stranger: no u r sure...
    (Bot): I see
    Stranger: r u m/f
    (Bot): That's disgusting!
    (stranger disconnected)

###Build

To build libomegle and the included demos, run `make libomegle demos` from the root directory.

To build the python bindings, run `make python-bindings`. You will need to have [swig](http://www.swig.org/).

Alternatively, download a [precompiled package](https://github.com/personalcomputer/libomegle/downloads).