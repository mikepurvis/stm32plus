/*
 * This file is a part of the open source stm32plus library.
 * Copyright (c) 2011,2012,2013,2014 Andy Brown <www.andybrown.me.uk>
 * Please see website for licensing terms.
 */

#include "config/stm32plus.h"
#include "config/usb/device/cdc.h"


using namespace stm32plus;
using namespace stm32plus::usb;


/**
 * Compatible MCU:
 *   STM32F4
 *
 * Tested on devices:
 *   STM32F407VGT6 / Windows 8.1 x64 host
 */

class UsbDeviceCdcComPortTest {

  public:

    /*
     * declare a type for the the USB stack
     */

    typedef ComPortCdcDevice<
      InternalFsPhy<>,                    // use the internal full speed PHY with no features
      ManufacturerTextFeature,            // we'll supply a manufacturer name
      ProductTextFeature,                 // ... and a product name
      SerialNumberTextFeature,            // ... and a serial number
      ConfigurationTextFeature            // ... and a config text string
    > MyUsb;


    /*
     * Flag to communicate between IRQ and non-IRQ code and the message to send back
     */

    volatile bool _responseReady;
    char _message[16];


    /*
     * Run the example
     */

    void run() {

      // initialise

      _responseReady=false;
      strcpy(_message,"You pressed: X\r\n");

      /*
       * set up the parameters for the USB CDC device. Do not attempt to reuse vid/pid combinations unless
       * you know how to flush your PC's USB driver cache because Windows caches the characteristics of each
       * device and will suspend your device if it suddenly re-appears as a different device type.
       */

      MyUsb::Parameters usbParams;

      usbParams.device_vid=0xDEAD;           // demo vendor ID
      usbParams.device_pid=0x0010;           // demo product ID

      usbParams.device_manufacturer_text="Andy's Workshop";   // see params.device_language_[ids/count] to change the languages
      usbParams.device_product_text="stm32plus virtual COM port";
      usbParams.device_serial_text="0123456789";
      usbParams.device_configuration_text="My configuration";

      usbParams.cdc_com_port_rx_buffer_size=16;  // default of 1Kb is far too big for this app

      /*
       * Declare the USB object - this will initialise internal variables but will not
       * start the peripheral
       */

      MyUsb usb;

      /*
       * Subscribe to errors
       */

      usb.UsbErrorEventSender.insertSubscriber(UsbErrorEventSourceSlot::bind(this,&UsbDeviceCdcComPortTest::onError));

      /*
       * Subscribe to USB events - data received from the host will be notified to us
       * asynchronously.
       */

      usb.UsbEventSender.insertSubscriber(UsbEventSourceSlot::bind(this,&UsbDeviceCdcComPortTest::onEvent));

      /*
       * Start the USB peripheral. It will run asynchronously. There is no requirement
       * for the parameters to remain in scope after the initialise call
       */

      if(!usb.initialise(usbParams))
        for(;;);      // onError() has already locked up

      // loop forever, or until an error interrupts us

      for(;;) {

        // block until a response is ready to send

        while(!_responseReady);
        _responseReady=false;

        // send it

        usb.transmit(_message,sizeof(_message));

        // we're ready to receive the next packet from the host

        usb.beginReceive();
      }
    }


    /**
     * Event callback from the USB stack. Lots of stuff will come through here but
     * we're only interested in data arriving from the host and control messages
     */

    void onEvent(UsbEventDescriptor& ued) {

      // reject all events that we don't care about

      if(ued.eventType==UsbEventDescriptor::EventType::CDC_DATA_RECEIVED)
        onData(static_cast<CdcDataReceivedEvent&>(ued));
      else if(ued.eventType==UsbEventDescriptor::EventType::CDC_CONTROL)
        onControl(static_cast<CdcControlEvent&>(ued));
    }


    /**
     * Control event received from the host
     */

    void onControl(CdcControlEvent& event) {
      //XXX: move txBusy to inEndpointFeatureBase
    }


    /**
     * Data received from the host
     */

    void onData(CdcDataReceivedEvent& event) {

      // add character to the message to send

      _message[13]=event.data[0];

      // signal to the main loop that a response is ready

      _responseReady=true;
    }


    /**
     * USB error event received
     * @param uee the event descriptor
     */

    void onError(UsbErrorEvent& uee) {

      // ignore unconfigured errors from the HID device

      if(uee.provider==ErrorProvider::ERROR_PROVIDER_USB_DEVICE && uee.code==MyUsb::E_UNCONFIGURED)
        return;

      // flash the RED led on PD5 at 1Hz

      GpioD<DefaultDigitalOutputFeature<5>> pd;

      for(;;) {
        pd[5].reset();
        MillisecondTimer::delay(500);
        pd[5].set();
        MillisecondTimer::delay(500);
      }
    }
};


/*
 * Main entry point
 */

int main() {

  // initialise the interrupt controller

  Nvic::initialise();

  // initialise the millisecond timer

  MillisecondTimer::initialise();

  // run the test

  UsbDeviceCdcComPortTest test;
  test.run();

  // not reached
  return 0;
}