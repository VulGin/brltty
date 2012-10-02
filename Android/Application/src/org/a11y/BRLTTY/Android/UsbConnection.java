/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2012 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

package org.a11y.BRLTTY.Android;

import java.util.HashMap;
import java.util.Iterator;

import android.util.Log;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;

import android.app.PendingIntent;

import android.hardware.usb.*;

public class UsbConnection {
  private final String LOG_TAG = this.getClass().getName();

  private static Context usbContext;
  private static UsbManager usbManager;

  public static void setContext (Context context) {
    usbContext = context;

    usbManager = (UsbManager)usbContext.getSystemService(Context.USB_SERVICE);
  }

  public static Iterator<UsbDevice> getDeviceIterator () {
    HashMap<String, UsbDevice> devices = usbManager.getDeviceList();
    return devices.values().iterator();
  }

  public static UsbDevice getNextDevice (Iterator<UsbDevice> iterator) {
    return iterator.hasNext()? iterator.next(): null;
  }

  private static final Object usbGlobalLock = new Object();

  private static final String ACTION_USB_PERMISSION = "org.a11y.BRLTTY.Android.USB_PERMISSION";
  private static BroadcastReceiver usbPermissionReceiver = null;
  private static PendingIntent usbPermissionIntent = null;

  private void ensurePermissionReceiver () {
    synchronized (usbGlobalLock) {
      if (usbPermissionReceiver == null) {
        usbPermissionReceiver = new BroadcastReceiver () {
          @Override
          public void onReceive (Context context, Intent intent) {
            String action = intent.getAction();

            if (action.equals(ACTION_USB_PERMISSION)) {
              synchronized (this) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                  Log.i(LOG_TAG, "permission granted for USB device: " + device);
                } else {
                  Log.w(LOG_TAG, "permission denied for USB device: " + device);
                }
              }
            }
          }
        };

        usbPermissionIntent = PendingIntent.getBroadcast(usbContext, 0, new Intent(ACTION_USB_PERMISSION), 0);
        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        usbContext.registerReceiver(usbPermissionReceiver, filter);
      }
    }
  }

  public UsbConnection () {
    ensurePermissionReceiver();
  }

  public boolean obtainPermission (UsbDevice device) {
    if (usbManager.hasPermission(device)) {
      return true;
    }

    usbManager.requestPermission(device, usbPermissionIntent);
    return usbManager.hasPermission(device);
  }
}
