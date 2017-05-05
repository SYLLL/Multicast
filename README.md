# Multicast
For 600.437 Distributed Systems

Use the spread to build a protocol that reliably sends messages between multiple
processors in the same spread network in agreed order. We have mcast.c and
net include.h.
In mcast.c, it takes num of messages, process index and num of processes as
inputs. num of messages indicates how many messages this processor needs to
send. process index is a unique number assigned to this processor. num of processes
indicates how many processors there should be in this membership. Each pro-
cessor in the end has an output le: process index.out that has all the received
messages in it.
