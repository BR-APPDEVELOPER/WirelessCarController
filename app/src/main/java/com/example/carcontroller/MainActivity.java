package com.example.carcontroller;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.MediaType;
import okhttp3.Response;
import okhttp3.FormBody;


public class MainActivity extends AppCompatActivity implements FetchData.DataListener {

    Button frontBtn, backBtn, leftBtn, rightBtn, startBtn;
    TextView responseMsgTv;
    EditText keyEdt;

    private static final String MOTOR_FORWARD_URL = "http://192.168.43.168/motor/forward";
    private static final String MOTOR_BACKWARD_URL = "http://192.168.43.168/motor/backward";
    private static final String MOTOR_RIGHT_DIRECTION_URL = "http://192.168.43.168/motor/turnright";
    private static final String MOTOR_LEFT_DIRECTION_URL = "http://192.168.43.168/motor/turnleft";
    private static final String CAR_START_KEY_URL = "http://192.168.43.168/startcar";

    private static final int INTERVAL = 1000; // 5 seconds
    private Handler handler;
    private Runnable runnable;

    private OkHttpClient client = new OkHttpClient();
    boolean flag = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        frontBtn = findViewById(R.id.forward_btn);
        backBtn = findViewById(R.id.backward_btn);
        leftBtn = findViewById(R.id.left_btn);
        rightBtn = findViewById(R.id.right_btn);
        responseMsgTv = findViewById(R.id.response_message_tv);
        keyEdt = findViewById(R.id.key_edt);
        startBtn = findViewById(R.id.start_btn);

        handler = new Handler();
        runnable = new Runnable() {
            @Override
            public void run() {
                new FetchData(MainActivity.this).execute();
                handler.postDelayed(this, INTERVAL);
            }
        };
        handler.post(runnable);

        startBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String key = startBtn.getText().toString();
                sendDataToESP32(CAR_START_KEY_URL, key);
            }
        });

        frontBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        sendRequest(MOTOR_FORWARD_URL);
                        return true;
                    case MotionEvent.ACTION_UP:
                        sendRequest(MOTOR_FORWARD_URL);
                        return true;
                }
                return false;
            }
        });

        backBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        sendRequest(MOTOR_BACKWARD_URL);
                        return true;
                    case MotionEvent.ACTION_UP:
                        sendRequest(MOTOR_BACKWARD_URL);
                        return true;
                }
                return false;
            }
        });

        rightBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        sendRequest(MOTOR_RIGHT_DIRECTION_URL);
                        return true;
                    case MotionEvent.ACTION_UP:
                        sendRequest(MOTOR_RIGHT_DIRECTION_URL);
                        return true;
                }
                return false;
            }
        });

        leftBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        sendRequest(MOTOR_LEFT_DIRECTION_URL);
                        return true;
                    case MotionEvent.ACTION_UP:
                        sendRequest(MOTOR_LEFT_DIRECTION_URL);
                        return true;
                }
                return false;
            }
        });
    }

    private void sendRequest(final String urlString) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    URL url = new URL(urlString);
                    HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();
                    try {
                        urlConnection.setRequestMethod("GET");
                        int responseCode = urlConnection.getResponseCode();
                        if (responseCode == 200) {
                            BufferedReader in = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
                            StringBuilder response = new StringBuilder();
                            String line;

                            while ((line = in.readLine()) != null) {
                                response.append(line);
                            }

                            in.close();
                            responseMsgTv.setText(response.toString());
                        } else {
                            //7Toast.makeText(MainActivity.this, "Error: " + responseCode, Toast.LENGTH_SHORT).show();
                            Log.d("ERROR", "" + responseCode);
                        }
                    } finally {
                        urlConnection.disconnect();
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    private void sendDataToESP32(String url, String data) {
        RequestBody body = new FormBody.Builder()
                .add("data", data)
                .build();

        Request request = new Request.Builder()
                .url(url)
                .post(body)
                .build();

        client.newCall(request).enqueue(new okhttp3.Callback() {
            @Override
            public void onFailure(okhttp3.Call call, IOException e) {
                e.printStackTrace();
            }

            @Override
            public void onResponse(okhttp3.Call call, Response response) throws IOException {
                if (!response.isSuccessful())
                    throw new IOException("Unexpected code " + response);

                // Log response (optional)
                Log.d("Response", response.body().string());
            }
        });
    }

    @Override
    public void onDataReceived(String data) {
        if (data != null) {
            if (data.toLowerCase().trim().equals("enabled")) {
                frontBtn.setEnabled(false);
                backBtn.setEnabled(false);
                leftBtn.setEnabled(false);
                rightBtn.setEnabled(false);
                responseMsgTv.setText("Emergency stop activated. Car will not run.");
            } else if (data.toLowerCase().trim().equals("disabled")) {
                frontBtn.setEnabled(true);
                backBtn.setEnabled(true);
                leftBtn.setEnabled(true);
                rightBtn.setEnabled(true);
                responseMsgTv.setText("Emergency stop deactivated. Car will run Now.");
            }

        } else {
            responseMsgTv.setText("Error fetching in Emergency data.");
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        handler.removeCallbacks(runnable);
    }
}