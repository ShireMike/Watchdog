Watchdog attempts to connect with a specified DNS server every 5 minutes.
If the connection fails for 'n' minutes continuously a relayed is activated.
This relay cuts power to the router for 30 seconds , causing it restart.
A notification email is sent when line is recovered.

Code is for a Beaglebone, but should run OK on a Raspberry Pi after altering
GPIO chip and pin numbers.
