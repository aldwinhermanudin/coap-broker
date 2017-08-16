# coap-broker: A C implementation of the CoAP Broker REST API [(draft-ietf-core-coap-pubsub-01)](https://tools.ietf.org/html/draft-ietf-core-coap-pubsub-01)

ABOUT COAP-BROKER
=============

coap-broker is an implementation of the publish-subscriber architecture used by the MQTT protocol in accordance with [draft-ietf-core-pubsub-01](https://tools.ietf.org/html/draft-ietf-core-coap-pubsub-01). In addition to using the publish-subscribe architecture, coap-broker has a feature to bridge the coap-broker with an MQTT Broker not found on [draft-ietf-core-pubsub-01](https://tools.ietf.org/html/draft-ietf-core-coap-pubsub-01). This implementation use the [libcoap](https://libcoap.net/) as a library that handles data transactions using the CoAP protocol and [paho.mqtt.c](https://Github.com/eclipse/paho.mqtt.c) acting as a bridge with MQTT Broker

PACKAGE CONTENTS
================

This directory contains the source code of coap-broker. The misc directory contains experimental codes used during development stage,a library dependencies used by the coap-broker, testing script to load test the coap-broker, raspberry pi startup script to configure the raspberry pi, and previous work of related project. 


ADDITIONAL INFORMATION
======================

The library was developed for the purpose of being implemented on a 6LoWPAN network with Raspberry Pi Zero as a processing unit using the MRF24J40MA transceiver. However, this library can be used on any IP network.
