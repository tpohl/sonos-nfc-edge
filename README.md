# Sonos-nfc-edge

Particle Edge Device for the Sonos NFC System.

## Overview

System Purpose:
I want to create NFC Tags that trigger playback of something on a SONOS Speaker.

Architecture:

* Speaker: Any Sonos Speaker will do
* Playables: All things that I will play are SONOS Playlists
* Controller: a Raspberry PI or similar Computer running the Sonos-HTTP-API (https://github.com/jishi/node-sonos-http-api)
* Edge: A Particle (https://www.particle.io) Photon with a MFRC522 NFC Reader and an RGB (common cathode) LED
* Lots of Tags (NTAG213)

## Assembly:

* Wire up the MFRC522 to the Photon and optionally add the LED.
* Flash the Software on it and use your Particle Cloud Console to read / write tags and set the URLs and Room Name (your speaker name)

## Flashing

Please flash a NFC Text file with the device or your Phone on the NFC Tags. 

Please prefix the Playlist name with "p/" -> if your Playlist is called "ROCKOUT" flash "p/ROCKOUT". 
