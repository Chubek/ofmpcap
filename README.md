OFM-PCAP is a daemon which allows the user to constantly remain in touch with devices connected to the same router as they are connected to --- and furthermore it alows them to pull data from these routers. 


OFM-PCAP as mentioned before is a daemon so it is constantly running. You can send requests to OFM-PCAP by writing to one of the IPC methods its connected to, and interface with it via its language which is very simple to use.

The language is a plain command language. Via it you may request:

* Listing all the devices connected to your router
* Send and receive packets to these devices
* Create a loopback interface to tunnel through these devices
* Send these packets to a third party analyzier
* And more



### How does OFM-PCAP Work?

For the most part, OFM-PCAP relies on LibPCAP --- but it also uses AFM and `IfAddrs.h`.
