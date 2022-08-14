
package com.silead.frrfar;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.widget.ProgressBar;

public class NumberProgressBar extends ProgressBar {
    String mText;
    Paint mPaint;

    public NumberProgressBar(Context context) {
        super(context);
        initPaint();
    }

    public NumberProgressBar(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initPaint();
    }

    public NumberProgressBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        initPaint();
    }

    @Override
    public synchronized void setProgress(int progress) {
        setText(progress);
        super.setProgress(progress);
    }

    @Override
    protected synchronized void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        Rect rect = new Rect();
        this.mPaint.getTextBounds(mText, 0, mText.length(), rect);
        int x = (getWidth() / 2) - rect.centerX();
        int y = (getHeight() / 2) - rect.centerY();
        mPaint.setTextSize(getMeasuredHeight() / 10 * 8);
        canvas.drawText(mText, x, y, mPaint);
    }

    private void initPaint() {
        mPaint = new Paint();
        mPaint.setFlags(Paint.ANTI_ALIAS_FLAG);
        mPaint.setColor(Color.RED);
    }

    private void setText(int progress) {
        this.mText = String.valueOf(progress) + "/" + String.valueOf(this.getMax());
    }
}
