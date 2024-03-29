/*
  * @author STMicroelectronics MMY Application team
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2017 STMicroelectronics</center></h2>
  *
  * Licensed under ST MIX_MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/Mix_MyLiberty
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
*/

package com.st.st25nfc.type5.st25tv;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import com.google.android.material.navigation.NavigationView;
import androidx.core.view.GravityCompat;
import androidx.viewpager.widget.ViewPager;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.appcompat.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import com.st.st25sdk.Helper;
import com.st.st25nfc.generic.MainActivity;
import com.st.st25nfc.R;
import com.st.st25sdk.STException;
import com.st.st25sdk.type5.st25tv.ST25TVTag;
import com.st.st25nfc.generic.STFragmentActivity;
import com.st.st25nfc.generic.SlidingTabLayout;
import com.st.st25nfc.generic.STFragment;
import com.st.st25nfc.generic.STPagerAdapter;
import com.st.st25nfc.generic.util.UIHelper;
import com.st.st25nfc.generic.util.UIHelper.STFragmentId;

import java.util.ArrayList;
import java.util.List;

public class ST25TVActivity extends STFragmentActivity
        implements NavigationView.OnNavigationItemSelectedListener, STFragment.STFragmentListener {

    // Set here the Toolbar to use for this activity
    private int toolbar_res = R.menu.toolbar_empty;

    final static String TAG = "ST25TVActivity";
    public ST25TVTag mST25TVTag;

    STPagerAdapter mPagerAdapter;
    ViewPager mViewPager;

    private SlidingTabLayout mSlidingTabLayout;
    private SharedPreferences mSharedPreferences;

    ListView lv;

    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.pager_layout);

        if (super.getTag() instanceof ST25TVTag) {
            mST25TVTag = (ST25TVTag) super.getTag();
        }
        if (mST25TVTag == null) {
            showToast(R.string.invalid_tag);
            goBackToMainActivity();
            return;
        }

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setTitle(mST25TVTag.getName());

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer);
        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        drawer.setDrawerListener(toggle);
        toggle.syncState();

        NavigationView navigationView = (NavigationView) findViewById(R.id.navigation_view);
        navigationView.setNavigationItemSelectedListener(this);
        mMenu.inflateMenu(navigationView);

        List<STFragmentId> fragmentList = new ArrayList<STFragmentId>();

        fragmentList.add(UIHelper.STFragmentId.TAG_INFO_FRAGMENT_ID);
        fragmentList.add(UIHelper.STFragmentId.NDEF_DETAILS_FRAGMENT_ID);
        fragmentList.add(UIHelper.STFragmentId.CC_FILE_TYPE5_FRAGMENT_ID);
        fragmentList.add(UIHelper.STFragmentId.SYS_FILE_TYP5_FRAGMENT_ID);
        fragmentList.add(UIHelper.STFragmentId.RAW_DATA_FRAGMENT_ID);

        mPagerAdapter = new STPagerAdapter(getSupportFragmentManager(), getApplicationContext(), fragmentList);

        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mPagerAdapter);

        mSlidingTabLayout = (SlidingTabLayout) findViewById(R.id.sliding_tabs);
        mSlidingTabLayout.setViewPager(mViewPager);

        mSharedPreferences = PreferenceManager.getDefaultSharedPreferences(getBaseContext());

        // Check if the activity was started with a request to select a specific tab
        Intent mIntent = getIntent();
        int tabNbr = mIntent.getIntExtra("select_tab", -1);
        if(tabNbr != -1) {
            mViewPager.setCurrentItem(tabNbr);
        }

        Bundle bundle = getIntent().getExtras();
        if (bundle != null) {
            boolean isANewTag = bundle.getBoolean(MainActivity.NEW_TAG);
            if (isANewTag) {
                // We are entering to this activity to display a new tag.
                // Check if EAS alarm is set for this tag
                boolean checkEasAlarm = mSharedPreferences.getBoolean(ST25TVEasActivity.ST25TV_CHECK_EAS_ALARM_KEY, true);
                if (checkEasAlarm) {
                    checkEasAlarmStatus();
                }
            }
        }
    }

    private void checkEasAlarmStatus() {
        new Thread(new Runnable() {
            public void run() {
                // Warning: This function will be blocking until the information is got
                boolean isEasEnabled = mST25TVTag.isEasEnabled();
                Log.d(TAG, "isEasEnabled = " + isEasEnabled);
                if(isEasEnabled) {
                    // If the current activity is still active, display the EAS Alarm
                    if (mIsActive) {
                        processEasAlarm();
                    }
                }
            }
        }).start();
    }

    public void processEasAlarm() {
        new Thread(new Runnable() {
            public void run() {
                Log.v(TAG, "EAS Alarm !");

                // Retrieve EAS info
                try {
                    int easId = mST25TVTag.readEasId();
                    String telegram = mST25TVTag.readEasTelegram();
                    displayEasAlert(easId, telegram);

                } catch (STException e) {
                    e.printStackTrace();
                    showToast(R.string.eas_alarm_failed_to_read_id_or_telegram);
                }
            }
        }).start();
    }

    public void displayEasAlert(final int easId, final String telegram) {
        runOnUiThread(new Runnable() {
            public void run() {
                final AlertDialog alertDialog = new AlertDialog.Builder(ST25TVActivity.this).create();

                // set title
                alertDialog.setTitle(getString(R.string.eas_alarm));

                // inflate XML content
                View dialogView = getLayoutInflater().inflate(R.layout.fragment_st25tv_eas_alarm, null);
                alertDialog.setView(dialogView);

                Button easButton = (Button) dialogView.findViewById(R.id.goToEasMenuButton);
                easButton.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        // Leave the dialog box
                        alertDialog.dismiss();

                        // and start an activity
                        Intent intent = new Intent(ST25TVActivity.this, ST25TVEasActivity.class);
                        startActivity(intent);
                    }
                });


                TextView easIdTextView = (TextView) dialogView.findViewById(R.id.easIdTextView);
                easIdTextView.setText(Helper.convertIntToHexFormatString(easId));

                TextView easTelegramTextView = (TextView) dialogView.findViewById(R.id.easTelegramTextView);
                easTelegramTextView.setText(telegram);

                // show it
                alertDialog.show();
            }
        });

    }

    @Override
    public void onBackPressed() {
        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer);
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
        finish();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds read_list_items to the action bar if it is present.
        getMenuInflater().inflate(toolbar_res, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long

        // as you specify a parent activity in AndroidManifest.xml.


        return super.onOptionsItemSelected(item);
    }

    void processIntent(Intent intent) {
        Log.d(TAG, "Process Intent");
    }

    @SuppressLint("MissingSuperCall")
    @Override
    public void onNewIntent(Intent intent) {
        // onResume gets called after this to handle the intent
        setIntent(intent);
    }

    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle navigation view item clicks here.
        return mMenu.selectItem(this, item);
    }

    public ST25TVTag getTag() {
        return mST25TVTag;
    }

}

