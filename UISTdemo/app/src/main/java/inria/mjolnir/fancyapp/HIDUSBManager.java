package inria.mjolnir.fancyapp;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.util.Log;

/**
 * Created by Sebastien Poulmane & Matthieu Falce on 19/03/2017.
 * Modified by Sebastien Poulmane on 10/10/2017. HID-USB manager holds the times.
 */

class HIDUSBManager {

    private static final String TAG = "FancyApp-HIDUSBManager";

    static final boolean LAGMETER_EMULATOR = false;

    private static final int TIMEOUT_BULK_MS = 200;
    private static final String ACTION_USB_PERMISSION = "inria.mjolnir.fancyapp.USB_PERMISSION";
    static final String EXTRA_USB_PERMISSION = "usb-permission";

    private static String errorMessage;

    private UsbDeviceConnection mUsbConnection;
    private UsbEndpoint mEndpointHID;
    private byte[] dataBuffer;
    private long[] times;
    private boolean state;

    static HIDUSBManager getHIDUSBManager(Context context) {
        if (LAGMETER_EMULATOR) {
            errorMessage = "Lagmeter emulator enabled.";
            return new HIDUSBManager(null, null);
        }

        UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        UsbDeviceConnection connection;
        UsbDevice device = selectUsbDevice(manager, 0);
        UsbEndpoint endpoint;
        if (device != null)
            connection = manager.openDevice(device);
        else {
            errorMessage = "Can't open device";
            return null;
        }
        if (connection != null)
            endpoint = claimEndpointHID(device, connection);
        else {
            errorMessage = "Can't connect to device";
            return null;
        }
        if (endpoint == null) {
            errorMessage = "Can't claim endpoint HID";
            return null;
        }

        return new HIDUSBManager(connection, endpoint);
    }

    boolean read() {
        if (LAGMETER_EMULATOR)
            return true;

         if (mUsbConnection.bulkTransfer(mEndpointHID, dataBuffer, dataBuffer.length, TIMEOUT_BULK_MS) >= 0) {
            char source = (char)dataBuffer[2];
            if (state && source == 'D') {
                this.setTimestamp(TimeTag.LED, System.currentTimeMillis());
                state = false;
            }
            else if (source == 'P') {
                this.setTimestamp(TimeTag.PIEZZO, System.currentTimeMillis());
                state = true;
            }
            else
                return false;
        }
        else
            return false;

        return true;
    }

    void close() {
        mUsbConnection.close();
    }

    private static final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    Intent response = new Intent(FancyActivity.PACKAGE_NAME + FancyActivity.INTENT_SERVICE_NAME);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        response.putExtra(EXTRA_USB_PERMISSION, true);
                        Log.i(TAG, "USB permission granted");
                    }
                    else {
                        response.putExtra(EXTRA_USB_PERMISSION, false);
                        Log.e(TAG, "USB permission denied");
                    }
                    context.sendBroadcast(response);
                }
            }
        }
    };

    static void requestPermission(Context context) {
        UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        UsbDevice device = selectUsbDevice(manager, 0);
        if (device != null) {
            PendingIntent mPermissionIntent = PendingIntent.getBroadcast(context, 0, new Intent(ACTION_USB_PERMISSION), 0);
            IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
            context.registerReceiver(mUsbReceiver, filter);
            manager.requestPermission(device, mPermissionIntent);
        }
    }

    static String getErrMsg() {
        return HIDUSBManager.errorMessage;
    }

    private static UsbDevice selectUsbDevice(UsbManager manager, int index)
    {
        int size = manager.getDeviceList().size();
        if (size == 0 || index < 0 || index >= size) {
            Log.e(TAG, "Select USB device: Invalid index");
            return null;
        }
        return (UsbDevice)manager.getDeviceList().values().toArray()[index];
    }

    private static UsbEndpoint claimEndpointHID(UsbDevice device, UsbDeviceConnection connection) {
        UsbInterface itf = null;
        UsbEndpoint ept = null;
        for (int i = 0; i < device.getInterfaceCount(); i++) {
            if (device.getInterface(i).getInterfaceClass() == UsbConstants.USB_CLASS_HID) {
                itf = device.getInterface(i);
                ept = itf.getEndpoint(0);
            }
        }
        if (itf == null)
            Log.e(TAG, "Can't find HID interface");
        else {
            if (connection.claimInterface(itf, true))
                Log.i(TAG, "HID interface claimed");
            else
                Log.e(TAG, "Can't claim HID interface");
        }

        return ept;
    }

    private HIDUSBManager(UsbDeviceConnection connection, UsbEndpoint endpoint) {
        mUsbConnection = connection;
        mEndpointHID = endpoint;
        if (LAGMETER_EMULATOR)
            dataBuffer = null;
        else
            dataBuffer = new byte[mEndpointHID.getMaxPacketSize()];
        times = new long[TimeTag.length];
        state = false;
        for (int i = 0; i < times.length; i++)
            times[i] = -1;
    }

    void setTimestamp(TimeTag tag, long ms) {
        if (LAGMETER_EMULATOR) {
            long emulatedLatency;
            if (tag == TimeTag.TOUCH || tag == TimeTag.PIEZZO) {
                emulatedLatency = (long) (Math.random() * 100);
                this.times[TimeTag.PIEZZO.getValue()] = ms - emulatedLatency;
                this.times[TimeTag.TOUCH.getValue()] = ms;
                this.state = true;
            }
            else if (this.state && (tag == TimeTag.DISPLAY || tag == TimeTag.LED)) {
                emulatedLatency = (long) (Math.random() * 10);
                this.times[TimeTag.DISPLAY.getValue()] = ms;
                this.times[TimeTag.LED.getValue()] = ms + emulatedLatency;
                this.state = false;
            }
        }
        else
            this.times[tag.getValue()] = ms;
    }

    long getLatency(TimeTag start, TimeTag end) {
        long tStart = this.times[start.getValue()];
        long tEnd = this.times[end.getValue()];
        return (tStart > -1 && tEnd > -1) ? (tEnd - tStart) : -1;
    }

    void resetTimes() {
        for (int i = 0; i < times.length; i++)
            times[i] = -1;
    }
}
