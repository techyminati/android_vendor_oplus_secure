
package com.silead.frrfar;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;

public class LongClickButton extends Button {
    private long mDelayMillis = 100;
    private boolean isMotionEventUp = true;

    public LongClickButton(Context context, AttributeSet attrs) {
        super(context, attrs);
        initListener();
    }

    public LongClickButton(Context context) {
        super(context);
        initListener();
    }

    public LongClickButton(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initListener();
    }

    public void initListener() {
        this.setOnLongClickListener(new OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                isMotionEventUp = false;
                mHandler.sendEmptyMessage(0);
                return false;
            }
        });
        this.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_UP) {
                    isMotionEventUp = true;
                }
                return false;
            }
        });
    }

    Handler mHandler = new Handler() {
        public void handleMessage(android.os.Message msg) {
            if (!isMotionEventUp && isEnabled()) {
                performClick();
                mHandler.sendEmptyMessageDelayed(0, mDelayMillis);
            }
        };
    };

    public void setmDelayMillis(long delayMillis) {
        this.mDelayMillis = delayMillis;
    }
}