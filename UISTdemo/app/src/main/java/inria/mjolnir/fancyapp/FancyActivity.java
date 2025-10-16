package inria.mjolnir.fancyapp;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;


public class FancyActivity extends Activity {

    public final static String PACKAGE_NAME = "inria.mjolnir.fancyapp";
    public final static String INTENT_SERVICE_NAME = ".service";
    public final static String EXTRA_UPDATE_LOG = "log-update";

    private boolean isRunning;
    private HIDUSBManager lagmeter;
    private long[] lastLatencies;
    private MyGame myGame;

    private TextView tvPhotodiode;
    private TextView tvMessage;
    private TextView[] tvLatencies;

    private BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals (PACKAGE_NAME + INTENT_SERVICE_NAME)) {
                if (intent.hasExtra(HIDUSBManager.EXTRA_USB_PERMISSION)) {
                    if (intent.getBooleanExtra(HIDUSBManager.EXTRA_USB_PERMISSION, false)) {
                        Toast.makeText(context, "HID device detected and permission granted", Toast.LENGTH_SHORT).show();
                        lagmeter = HIDUSBManager.getHIDUSBManager(context);
                        if (lagmeter == null) {
                            Toast.makeText(context, "Can't connect to HID device. Close this app and try again.", Toast.LENGTH_SHORT).show();
                            tvMessage.setText(HIDUSBManager.getErrMsg());
                        } else {
                            tvMessage.setText(R.string.msg_running);
                            myGame.setLagmeter(lagmeter);
                            isRunning = true;
                            run.start();
                        }
                    }
                    else
                        Toast.makeText(context, "HID device not detected or permission not granted", Toast.LENGTH_LONG).show();
                } else if (intent.hasExtra(EXTRA_UPDATE_LOG)) {
                    lastLatencies[0] = lagmeter.getLatency(TimeTag.PIEZZO, TimeTag.TOUCH);
                    lastLatencies[1] = lagmeter.getLatency(TimeTag.TOUCH, TimeTag.LED);
                    lastLatencies[2] = lagmeter.getLatency(TimeTag.PIEZZO, TimeTag.LED);
                    for (int i = 0; i < lastLatencies.length; i++)
                        tvLatencies[i].setText(lastLatencies[i] >= 0 ? String.valueOf(lastLatencies[i]) : "-");
                }
            }
            else if (intent.getAction().equals("android.hardware.usb.action.USB_DEVICE_ATTACHED")) {
                HIDUSBManager.requestPermission(getApplicationContext());
                tvMessage.setText(R.string.msg_waiting);
            }
        }
    };

    private Thread run = new Thread(new Runnable() {
        @Override
        public void run() {
            while (isRunning) {
                if (lagmeter.read()) {
                    if ((lagmeter.getLatency(TimeTag.PIEZZO, TimeTag.LED) >= 0) && (lagmeter.getLatency(TimeTag.PIEZZO, TimeTag.TOUCH) >= 0)) {
                        Intent updateMessage = new Intent(PACKAGE_NAME + INTENT_SERVICE_NAME);
                        updateMessage.putExtra(EXTRA_UPDATE_LOG, true);
                        getApplicationContext().sendBroadcast(updateMessage);
                    }
                }
            }
        }
    });

    private View.OnTouchListener tapListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                if (lagmeter != null)
                    lagmeter.setTimestamp(TimeTag.TOUCH, System.currentTimeMillis());
                tvPhotodiode.setBackgroundColor(Color.WHITE);
                myGame.jump();
            }
            else if (event.getAction() == MotionEvent.ACTION_UP)
                tvPhotodiode.setBackgroundColor(Color.BLACK);
            return true;
        }
    };

    private SeekBar.OnSeekBarChangeListener seekListener = new SeekBar.OnSeekBarChangeListener() {
        @Override
        public void onProgressChanged(SeekBar sb, int value, boolean user) {
            if (user && myGame != null) {
                myGame.setGameSpeed(value);
            }
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
            // Do nothing
        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
            // Do nothing
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_fancy);

        tvPhotodiode = (TextView)findViewById(R.id.photodiode);
        tvPhotodiode.setBackgroundColor(Color.BLACK);
        tvMessage = (TextView)findViewById(R.id.latency);
        lastLatencies = new long[3];
        tvLatencies = new TextView[3];
        tvLatencies[0] = (TextView)findViewById(R.id.latency_e2e);
        tvLatencies[1] = (TextView)findViewById(R.id.latency_p2t);
        tvLatencies[2] = (TextView)findViewById(R.id.latency_sys);
        myGame = (MyGame)findViewById(R.id.space);
        myGame.setOnTouchListener(tapListener);
        SeekBar sbTap = (SeekBar)findViewById(R.id.seekBar);
        sbTap.setOnSeekBarChangeListener(seekListener);
        myGame.setGameSpeed(sbTap.getProgress());

        registerReceiver(receiver, new IntentFilter(PACKAGE_NAME + INTENT_SERVICE_NAME));
        registerReceiver(receiver, new IntentFilter("android.hardware.usb.action.USB_DEVICE_ATTACHED"));

        if (HIDUSBManager.LAGMETER_EMULATOR) {
            lagmeter = HIDUSBManager.getHIDUSBManager(getApplicationContext());
            tvMessage.setText(HIDUSBManager.getErrMsg());
            myGame.setLagmeter(lagmeter);
            isRunning = true;
            run.start();
        }
        else
            HIDUSBManager.requestPermission(getApplicationContext());
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        isRunning = false;
        try {
            run.join();
        } catch (InterruptedException e) {
            e.printStackTrace(); // Can't be seen, but it's there.
        }
        if (lagmeter != null)
            lagmeter.close();
    }

    public void resetTimes(View v) {
        if (lagmeter != null)
            lagmeter.resetTimes();
    }
}
