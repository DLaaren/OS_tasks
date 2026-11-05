# Network

_Tools are optional, but a simulator (e.g., Cisco Packet Tracer) is encouraged_

You are a junior network administrator hired by the recently opened Maple Street Coworking Café — a small coworking space with a café, a staff office, and guest Wi-Fi.

## Client structure
The customer has two zones:
1. __Cafe area__
- 2 payment terminals (POS)
- 2 smart coffee machines (IoT)
- 1 Wi-Fi access point — used by both staff and guests
2. __Staff Office__
- 3 office PCs
- 3 laptops
- 1 shared printer
- 1 small local server (accounting + business data)

## Expected result
You have to submit a Network Hierarchy Blueprint -- internet edge, routers, wireless access points, end devices

## Level 1 -- Foundation
Show you understand what each device is and where it belongs.

### Tasks
1. List every device in the café and classify it as business-critical, staff, guest, IoT, or shared peripheral.
2. Answer:
- Which devices should never be reachable from guest Wi-Fi? Why?
- Why is a shared Wi-Fi AP for staff and guests a problem?

## Level 2 -- Design
Produce a layered hierarchy with zones, VLANs, and IP plan.

### Tasks
1. Draw the hierarchy diagram with these layers labelled:
Internet edge
Router
Switch (opt.)
Access layer (access switch + Wi-Fi AP)
End devices
2. Complete the IP / VLAN plan.
For example:
```text
 VLAN   |  Name   |     Subnet         |     Gateway     |     DHCP     |    Devices
-------------------------------------------------------------------------------------------
 40     |   IOT   |   192.168.40.0/24  |  192.168.40.1   |    static    |   coffee machines  
```

## Level 3 -- Defend
Prove the security and failure recovery

### Tasks
1. Failure scenarios, describe cause/effect/fix:
- Guest device is accidentally placed on VLAN 20.
- POS terminal loses its default gateway.
- The Wi-Fi AP dies.
2. Design defense: in several sentences, justify the hierarchy and its security.
