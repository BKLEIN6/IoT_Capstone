#IoT Bootcamp Capstone

Overview
The purpose of this capstone is to be able to create a portable mesh network that can be deployed as needed in areas with minimal or no Cell signal. This version of the mesh network will be able to have a remote unit find it's GPS location and send it to a Ground Station. There is a 3rd node that will automatically allow the two devices to connect and continue to communicate once they are out of range.
Details
The device will use the following components: 
**Particle Argon (qty 1) 
**Teensy 3.1 (qty 1)
**ElectroPeak 64x128 OLED screen (qty 2) 
**resistor (1) 220ohm  
 **LED(qty 1)
 **Adafruit - Ultimate GPS (qty 1)
 **Sparkfun - XBEE Explorer (qty 2) 
**Digi XBEE Pro 900HP Dev Kit (qty 1)
 **Momentary Push Button (qty 1)
 **Breadboard (qty 2)
**External USB Power Packs (qty 3)
The project will be completed in the following steps:

    Notebook - rough documentation of idea and flow chart
    Fritzing - A diagram
    Breadboard
    Coding

Summary
This project is a proof of concept for adding onto a former cohorts idea for their Search and Rescue product design. The purpose of this capstone is to show that a portable mesh network can be set up to transmit data over extensive areas where cellphone signal is lacking and other elements may prevent good signal transmission, such as geography and vegetation.
There are 3 components being made for this project.
1) GPS Remote - This is unit will capture live GPS data, convert it to an XBEE API frame,  and transmit it to a specifically addressed unit (Ground Station). This unit will not create a network and will have to automatically join a specific network.
2) Ground Station - This unit will receive the data from the GPS Remote. This unit will create a network for the GPS Remote and Mesh Node to join. This unit will deconstruct the API frame it received, display the GPS coordinates on an OLED, and sent this information to an Adafruit Dashboard.
3) Mesh Node - This unit will only have power and will not be controlled by an external micro-controller. This node will only have its settings changed to be a router. It will only forward/repeat frames received. It does not save data. This unit should automatically connect the GPS Remote and Ground Station once they are out of range of each other and the Mesh Node is placed in between them. 

My Hackster
https://www.hackster.io/bklein6/portable-mesh-network-fb0e50


