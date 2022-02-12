# ESPHome Components
Custom components for [ESPHome](https://esphome.io/)

## Nanoview Sender
This component requires a custom arduino board to work. The sender replaces the [Nanoview Electricity Monitor](http://www.nanoview.co.za/protocol.html) display unit and allows the readings taken by the CT's to be sent to [Home Assistant](https://www.home-assistant.io/). Note that the Nanoview electricity monitor is built in South Africa and may not be available in other countries. The Nanoview needs to be installed into the electricity distribution board of your home by a competent electrician along with upto 16 CT sensors to monitor the power usage for each circuit of the home, the first of which monitors the main circuit breaker, so sends the total power consumption for the household.

At this pint, this initial version sets up entities in [Home Assistant](https://www.home-assistant.io/) where they can be displayed on the built in dashboards.

For details of configuration and how to build the Arduino board, see the [Nanoview Monitor docs](docs/nanoview_monitor/README.md).

