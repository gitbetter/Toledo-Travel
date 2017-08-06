COP 4338: Toledo Travel
=======================

This is a simple command line client/server application for a fictional travel agency called **Toledo Travel**, written in C using TCP sockets and the pthreads library for parallel processing.
A single instance of the server is needed to service clients, which can be started using

```
bin/TServer IP StartingPort NumberOfPorts InputDataFile [OutputDataFile]
```

where most of the arguments are self-explanatory. The only optional argument is **OutputDataFile**. If this argument is omitted, the flight information will be written back into **InputDataFile**
on server exit. A client can be started using

```
bin/TClient IP [Port]
```

If Port is ommitted, a random port (agent) will be assigned to handle the client's requests on the server side. The server is allowed a maximum of 8 ports (clients), but any number in between
can be specified using the **NumberOfPorts** argument for the client.

On startup, both the client and the server will display a set of available commands.

Once a client logs in to the server using _LOGON_, they can query flight information using the available commands or enter the global chat room using _CHAT_. This will lock the client into
chat mode until they execute _CHAT_ _EXIT_ to return to the agency home.

A client spawns its own thread on the server upon logging in, and all commands will run in parallel with those of other clients.

A demo of the application is on YouTube, viewable on [YouTube](https://youtu.be/1IVeCxCi4wM)
