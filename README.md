libomegle Readme
================

libomegle is a little C++ library for interfacing with the 'Omegle' <http://omegle.com/> random-pairing instant messaging service.

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

To build libomegle and the included demos, run `make` from the root directory.

The demo omegle-cli-client depends upon C++11 threads. If it fails to build, consider updating g++, or don't worry very much about it, it won't break the library build.