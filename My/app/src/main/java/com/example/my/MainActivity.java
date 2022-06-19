package com.example.my;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Typeface;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CalendarView;
import android.widget.TextView;

import com.example.my.databinding.ActivityMainBinding;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'my' library on application startup.
    static {
        //System.loadLibrary("my");
        System.loadLibrary("sealc");
    }

    private ActivityMainBinding binding;
    private Button button_send,button_cancel;
    CalendarView calendar;
    int y ,d,m;
    int y1 ,d1,m1;
    private int cnt_button = 0;

    private void init_but(){
        cnt_button =0;
        button_send.setText("choose start date");
    }

    public long getMilliFromDate(String dateFormat) {
        Date date = new Date();
        SimpleDateFormat formatter = new SimpleDateFormat("dd/MM/yyyy");
        try {
            date = formatter.parse(dateFormat);
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return date.getTime();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        button_send = (Button) findViewById(R.id.button6);
        button_cancel = (Button) findViewById(R.id.button7);
        init_but();
        TextView textView_c1 = (TextView)findViewById(R.id.textView2);
        TextView textView_c2 = (TextView)findViewById(R.id.textView4);
        TextView textView_uk = (TextView)findViewById(R.id.textView);



        button_send.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                cnt_button++;
                if(cnt_button==1){

                    button_send.setText("choose end date");
                }
                else
                {
                    String dayone = String.valueOf(d)+"/"+String.valueOf(m+1)+"/"+String.valueOf(y);
                    String daytow = String.valueOf(d1)+"/"+String.valueOf(m1+1)+"/"+String.valueOf(y1);

                    String str = strin(getMilliFromDate(dayone), getMilliFromDate(daytow));
                    String[] arrOfStr = str.split(" ");
                    float Array[] = new float[4096];

                   for(int i=0;i<4096;i++)
                       Array[i] = Float.valueOf(arrOfStr[i]);
                   int cnt_class_1 = 0;
                   int cnt_class_2 = 0;
                   int cnt_unknown = 0;

                   for(int i=0 ;i<4096;i+=2){
                       if((Array[i]-1)>Array[i+1]){
                           cnt_class_1++;

                       }else if((Array[i+1]-1)>Array[i]){
                           cnt_class_2++;
                       }
                       else {
                           cnt_unknown++;
                       }

                   }
                   textView_c2.setTypeface(null, Typeface.BOLD);
                   textView_c2.setText("class 1 : "+ String.valueOf(cnt_class_1));
                    textView_c1.setTypeface(null, Typeface.BOLD);
                    textView_uk.setTypeface(null, Typeface.BOLD);
                   textView_uk.setText("class 2 : "+ String.valueOf(cnt_class_2));
                   textView_c1.setText("class uknown : "+ String.valueOf(cnt_unknown));
                    init_but();
                }
            }
        });

        calendar = (CalendarView) findViewById(R.id.calendarView2);

        calendar.setOnDateChangeListener(new CalendarView.OnDateChangeListener() {

            @Override
            public void onSelectedDayChange(CalendarView view, int year, int month,int dayOfMonth) {
                if(cnt_button==0){
                    y = year;
                    m = month;
                    d = dayOfMonth;
                }else{
                    y1 = year;
                    m1 = month;
                    d1 = dayOfMonth;
                }

            }
        });

        button_cancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                init_but();
            }
        });

    }
    /**
     * A native method that is implemented by the 'my' native library,
     * which is packaged with this application.
     */
    public native String strin(long day ,long day2);
}