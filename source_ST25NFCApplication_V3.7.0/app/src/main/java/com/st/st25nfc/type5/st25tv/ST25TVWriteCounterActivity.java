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

import android.os.Bundle;
import android.os.Handler;
import com.google.android.material.navigation.NavigationView;
import androidx.fragment.app.FragmentManager;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.appcompat.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.FrameLayout;
import android.widget.Switch;
import android.widget.TextView;

import com.st.st25nfc.R;
import com.st.st25nfc.generic.MainActivity;
import com.st.st25nfc.generic.PwdDialogFragment;
import com.st.st25nfc.generic.STFragmentActivity;
import com.st.st25nfc.generic.STType5PwdDialogFragment;
import com.st.st25nfc.generic.util.UIHelper;
import com.st.st25sdk.NFCTag;
import com.st.st25sdk.STException;
import com.st.st25sdk.type5.st25tv.ST25TVTag;


public class ST25TVWriteCounterActivity extends STFragmentActivity implements STType5PwdDialogFragment.STType5PwdDialogListener, NavigationView.OnNavigationItemSelectedListener {

    // Set here the Toolbar to use for this activity
    private int toolbar_res = R.menu.toolbar_empty;

    static final String TAG = "CounterActivity";
    private Handler mHandler;
    private ST25TVTag myTag;
    FragmentManager mFragmentManager;
    int configurationPasswordNumber;

    private static final int RESET_COUNTER_VALUE = 1;
    private static final int ENABLE_COUNTER = 2;
    private static final int DISABLE_COUNTER = 3;

    private int mCurrentAction;


    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.default_layout);

        // Inflate content of FrameLayout
        FrameLayout frameLayout=(FrameLayout) findViewById(R.id.frame_content);
        View childView = getLayoutInflater().inflate(R.layout.fragment_st25tv_write_counter, null);
        frameLayout.addView(childView);

        NFCTag nfcTag = MainActivity.getTag();
        if (nfcTag instanceof ST25TVTag) {
            myTag = (ST25TVTag) nfcTag;
        } else {
            showToast(R.string.invalid_tag);
            goBackToMainActivity();
            return;
        }

        configurationPasswordNumber = UIHelper.getConfigurationPasswordNumber(myTag);

        mHandler = new Handler();
        mFragmentManager = getSupportFragmentManager();

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer);
        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        drawer.setDrawerListener(toggle);
        toggle.syncState();

        NavigationView navigationView = (NavigationView) findViewById(R.id.navigation_view);
        navigationView.setNavigationItemSelectedListener(this);
        mMenu.inflateMenu(navigationView);

        Button resetCounterButton = (Button) findViewById(R.id.resetCounterButton);
        resetCounterButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                resetCounter();
            }
        });

    }

    @Override
    public void onResume() {
        super.onResume();

        getWriteCounterStateAndValue();
    }

    /**
     * Get current value of Write Counter.
     *
     * NB: The access to register values should be done in a background thread because, if the
     * cache is not up-to-date, it will trigger a read of register value from the tag.
     */
    private void getWriteCounterStateAndValue() {
        new Thread(new Runnable() {
            public void run() {
                try {
                    final boolean isWriteCounterEnabled = myTag.getRegisterCounterConfiguration().isCounterEnabled();
                    final int counterValue = myTag.readCounterValue();

                    // Post an action to UI Thead to update the widgets
                    runOnUiThread(new Runnable() {
                        public void run() {
                            TextView counterValueTextView = (TextView) findViewById(R.id.counterValueTextView);
                            counterValueTextView.setText(String.valueOf(counterValue));

                            Switch counterSwitch = (Switch) findViewById(R.id.counterSwitch);
                            counterSwitch.setChecked(isWriteCounterEnabled);

                            // Now that the Switch is initialized, we can set a listener to be
                            // notified when the switch is changing
                            counterSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                                    if (isChecked) {
                                        writeCounterState(true);
                                    } else {
                                        writeCounterState(false);
                                    }
                                }
                            });

                        }
                    });

                } catch (STException e) {
                    switch (e.getError()) {
                        case TAG_NOT_IN_THE_FIELD:
                            showToast(R.string.tag_not_in_the_field);
                            break;
                        default:
                            e.printStackTrace();
                            showToast(R.string.error_while_reading_the_tag);
                    }
                }
            }
        }).start();
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

    private void writeCounterState(final boolean enableCounter){
        new Thread(new Runnable() {
            public void run() {

                if(enableCounter) {
                    Log.v(TAG, "ENABLE_COUNTER");
                    mCurrentAction = ENABLE_COUNTER;
                } else {
                    Log.v(TAG, "DISABLE_COUNTER");
                    mCurrentAction = DISABLE_COUNTER;
                }

                try {
                    myTag.enableCounter(enableCounter);
                    showToast(R.string.tag_updated);

                } catch (STException e) {
                    switch (e.getError()) {
                        case TAG_NOT_IN_THE_FIELD:
                            showToast(R.string.tag_not_in_the_field);
                            break;
                        case CONFIG_PASSWORD_NEEDED:
                            //Log.v(TAG, "Password needed!");
                            displayPasswordDialogBox();
                            break;
                        default:
                            e.printStackTrace();
                            showToast(R.string.error_while_updating_the_tag);
                    }
                }
            }
        }).start();
    }

    private void resetCounter(){
        new Thread(new Runnable() {
            public void run() {
                Log.v(TAG, "resetCounter");
                mCurrentAction = RESET_COUNTER_VALUE;

                try {
                    myTag.resetCounter();
                    showToast(R.string.tag_updated);

                    // Refresh display
                    getWriteCounterStateAndValue();

                } catch (STException e) {
                    switch (e.getError()) {
                        case TAG_NOT_IN_THE_FIELD:
                            showToast(R.string.tag_not_in_the_field);
                            break;
                        case CONFIG_PASSWORD_NEEDED:
                            //Log.v(TAG, "Password needed!");
                            displayPasswordDialogBox();
                            break;
                        default:
                            e.printStackTrace();
                            showToast(R.string.error_while_updating_the_tag);
                    }
                }
            }
        }).start();
    }

    private void displayPasswordDialogBox() {
        Log.v(TAG, "displayPasswordDialogBox");

        // Warning: Function called from background thread! Post a request to the UI thread
        mHandler.post(new Runnable() {
            public void run() {
                STType5PwdDialogFragment pwdDialogFragment = STType5PwdDialogFragment.newInstance(
                        STType5PwdDialogFragment.STPwdAction.PRESENT_CURRENT_PWD,
                        configurationPasswordNumber,
                        getResources().getString(R.string.enter_configuration_pwd));
                if(pwdDialogFragment!=null) pwdDialogFragment.show(mFragmentManager, "pwdDialogFragment");
            }
        });
    }

    public void onSTType5PwdDialogFinish(int result) {
        Log.v(TAG, "onSTType5PwdDialogFinish. result = " + result);
        if (result == PwdDialogFragment.RESULT_OK) {
            // Config password has been entered successfully so we can now retry the command
            switch(mCurrentAction) {
                case RESET_COUNTER_VALUE:
                    resetCounter();
                    break;
                case ENABLE_COUNTER:
                    writeCounterState(true);
                    break;
                case DISABLE_COUNTER:
                    writeCounterState(false);
                    break;
            }
        } else {
            Log.e(TAG, "Action failed! Tag not updated!");
        }
    }

    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        return mMenu.selectItem(this, item);
    }
}
