
package com.silead.frrfar;

import android.app.Activity;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerFrrFarEnroll;

public class FingerSearchSensorActivity extends Activity implements View.OnClickListener {
    private FingerManager mFingerManager;

    private TextView mFingerSearchXDescTextView;
    private TextView mFingerSearchXTextView;
    private Button mFingerSearchXPlusBtn;
    private Button mFingerSearchXSubBtn;
    private TextView mFingerSearchYDescTextView;
    private TextView mFingerSearchYTextView;
    private Button mFingerSearchYPlusBtn;
    private Button mFingerSearchYSubBtn;

    private ImageView mFingerSearchImgView;
    private boolean mFingerSearchImgViewZoomIn = false;

    private View mFingerSearchAreaView;
    private boolean mFingerSearchAreaViewBgColor = false;

    private SensorImageView mSensorImgView;
    private Button mBackBtn;

    private FingerImageSize mFingerprintImageSize = new FingerImageSize();
    private int mFingerprintImageXCenter;
    private int mFingerprintImageYCenter;
    private String mFingerprintImageName = null;
    private int mFingerprintImageId;
    private WindowManager.LayoutParams lp;
    private DisplayMetrics dm;

    private boolean mFinish = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.finger_search_sensor_optic);

        mFingerSearchXDescTextView = (TextView) findViewById(R.id.finger_search_sensor_x_desc);
        mFingerSearchXTextView = (TextView) findViewById(R.id.finger_search_sensor_x);
        mFingerSearchXPlusBtn = (Button) findViewById(R.id.finger_search_sensor_x_plus_btn);
        mFingerSearchXSubBtn = (Button) findViewById(R.id.finger_search_sensor_x_sub_btn);
        mFingerSearchYDescTextView = (TextView) findViewById(R.id.finger_search_sensor_y_desc);
        mFingerSearchYTextView = (TextView) findViewById(R.id.finger_search_sensor_y);
        mFingerSearchYPlusBtn = (Button) findViewById(R.id.finger_search_sensor_y_plus_btn);
        mFingerSearchYSubBtn = (Button) findViewById(R.id.finger_search_sensor_y_sub_btn);

        mFingerSearchImgView = (ImageView) findViewById(R.id.finger_search_sensor_imageview);
        mFingerSearchImgView.setScaleType(ImageView.ScaleType.CENTER);

        mFingerSearchAreaView = (View) findViewById(R.id.finger_search_sensor_area);

        mFingerSearchImgView.setOnClickListener(this);
        mFingerSearchXDescTextView.setOnClickListener(this);
        mFingerSearchYDescTextView.setOnClickListener(this);
        mFingerSearchXTextView.setOnClickListener(this);
        mFingerSearchYTextView.setOnClickListener(this);

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
            if (!FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
                mBackBtn.setVisibility(View.GONE);
            }
        }

        if (mFingerSearchXPlusBtn != null) {
            mFingerSearchXPlusBtn.setOnClickListener(this);
        }
        if (mFingerSearchXSubBtn != null) {
            mFingerSearchXSubBtn.setOnClickListener(this);
        }
        if (mFingerSearchYPlusBtn != null) {
            mFingerSearchYPlusBtn.setOnClickListener(this);
        }
        if (mFingerSearchYSubBtn != null) {
            mFingerSearchYSubBtn.setOnClickListener(this);
        }

        mFingerManager = FingerManager.getDefault(this);

        getDefaultSizeAndPostion();
    }

    @Override
    public void onResume() {
        super.onResume();

        initIcon();
        showIcon();

        mFinish = false;
        testGetImage();

        updatePosition(0, 0);
    }

    @Override
    public void onPause() {
        super.onPause();

        hideIcon();

        mFinish = true;
        testFinish();

        finish();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        } else if (v == mFingerSearchXPlusBtn) {
            updatePosition(1, 0);
        } else if (v == mFingerSearchXSubBtn) {
            updatePosition(-1, 0);
        } else if (v == mFingerSearchYPlusBtn) {
            updatePosition(0, 1);
        } else if (v == mFingerSearchYSubBtn) {
            updatePosition(0, -1);
        } else if (v == mFingerSearchImgView) {
            mFingerSearchImgViewZoomIn = !mFingerSearchImgViewZoomIn;
            if (mFingerSearchImgViewZoomIn) {
                zoomView(mFingerSearchImgView, 1f, 1.5f, 1f, 1.5f);
            } else {
                zoomView(mFingerSearchImgView, 1.5f, 1f, 1.5f, 1f);
            }
        } else if (v == mFingerSearchXDescTextView) {
            if (mSensorImgView != null) {
                mSensorImgView.updateFun(this, SensorImageView.SEARCH_ICON_FLAG_DRAG);
            }
        } else if (v == mFingerSearchXTextView) {
            if (mSensorImgView != null) {
                mSensorImgView.updateFun(this, SensorImageView.SEARCH_ICON_FLAG_FINGER_ICON);
            }
        } else if (v == mFingerSearchYDescTextView) {
            Log.v(FingerSettingsConst.LOG_TAG, "update bg color");
            if (mFingerSearchAreaView != null) {
                mFingerSearchAreaViewBgColor = !mFingerSearchAreaViewBgColor;
                Log.v(FingerSettingsConst.LOG_TAG, "update bg color: " + mFingerSearchAreaViewBgColor);
                if (mFingerSearchAreaViewBgColor) {
                    mFingerSearchAreaView.setBackgroundColor(0xFF000000);
                } else {
                    mFingerSearchAreaView.setBackgroundColor(0x000000);
                }
            }
        } else if (v == mFingerSearchYTextView) {
            if (mSensorImgView != null) {
                mSensorImgView.updateFun(this, SensorImageView.SEARCH_ICON_FLAG_GRID);
            }
        }
    }

    private void testGetImage() {
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testGetImage: ");
        }

        if (mFingerManager != null) {
            mFingerManager.testGetImage(FingerSettingsConst.OTHER_GATHER_MODE_ORIG, -1, 0, mTestCmdCallback);
        }
    }

    public void testFinish() {
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testFinish: finish");
        }
        if (mFingerManager != null) {
            mFingerManager.testFinish(mTestCmdCallback);
        }
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_GET_IMAGE && result instanceof FingerFrrFarEnroll) {
                if (!mFinish) {
                    updateData(result);
                }
            }
        }
    };

    private void updateData(Object result) {
        if (result instanceof FingerFrrFarEnroll) {
            FingerFrrFarEnroll enrollResult = (FingerFrrFarEnroll) result;
            if (FingerSettingsConst.LOG_DBG) {
                Log.v(FingerSettingsConst.LOG_TAG, "updateData: " + enrollResult.toString());
            }

            if (enrollResult.getErrCode() >= FingerManager.TEST_RESULT_OK) {
                if (enrollResult.isImgOrig()) { // original image
                     Bitmap bitmapImage = BitmapFactory.decodeByteArray(enrollResult.getData(), 0, enrollResult.getData().length);
                     mFingerSearchImgView.setImageBitmap(bitmapImage);
                }
            }
        }
    }

    private void updatePosition(int xOffset, int yOffset) {
        if (mSensorImgView != null) {
            int imageRadius = mFingerprintImageSize.height / 2;
            mFingerprintImageYCenter += yOffset;
            if (mFingerprintImageYCenter < imageRadius) {
                mFingerprintImageYCenter = imageRadius;
            } else if (mFingerprintImageYCenter > dm.heightPixels - imageRadius) {
                mFingerprintImageYCenter = dm.heightPixels - imageRadius;
            }
            lp.y = mFingerprintImageYCenter - imageRadius;

            imageRadius = mFingerprintImageSize.width / 2;
            mFingerprintImageXCenter += xOffset;
            if (mFingerprintImageXCenter < imageRadius) {
                mFingerprintImageXCenter = imageRadius;
            } else if (mFingerprintImageXCenter > dm.widthPixels - imageRadius) {
                mFingerprintImageXCenter = dm.widthPixels - imageRadius;
            }
            lp.x = mFingerprintImageXCenter - imageRadius;
            WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
            wm.updateViewLayout(mSensorImgView, lp);

            if (mFingerSearchXTextView != null) {
                mFingerSearchXTextView.setText(Integer.toString(mFingerprintImageXCenter));
            }
            if (mFingerSearchYTextView != null) {
                mFingerSearchYTextView.setText(Integer.toString(mFingerprintImageYCenter));
            }
        }
    }

    private void getDefaultSizeAndPostion() {
        dm = getResources().getDisplayMetrics();

        mFingerprintImageYCenter = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_Y_NAME, FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_Y_DEFAULT);
        mFingerprintImageXCenter = dm.widthPixels/2;

        mFingerprintImageName = SharedPreferencesData.loadDataString(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_NAME);
        if (mFingerprintImageName == null || mFingerprintImageName.isEmpty()) {
            mFingerprintImageName = FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT;
        }
        mFingerprintImageId = FingerSettingsConst.getDrawableId(this, getPackageName(), mFingerprintImageName, FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT);
        FingerSettingsConst.getDrawableSize(this, mFingerprintImageId, mFingerprintImageSize);

        if (mFingerprintImageYCenter < mFingerprintImageSize.height / 2) {
            mFingerprintImageYCenter = mFingerprintImageSize.height / 2;
        }
        if (mFingerprintImageXCenter < mFingerprintImageSize.width / 2) {
            mFingerprintImageXCenter = mFingerprintImageSize.width / 2;
        }
    }

    private void zoomView(Object target, float srcScaleX, float destScaleX, float srcScaleY, float destScaleY) {
        Log.v(FingerSettingsConst.LOG_TAG, "srcScaleX:" + srcScaleX + ", destScaleX:" + destScaleX + ", srcScaleY:" + srcScaleY + ", destScaleY:" + destScaleY);
        AnimatorSet animator = new AnimatorSet();
        animator.playTogether(
            ObjectAnimator.ofFloat(target, "scaleX", srcScaleX, destScaleX),
            ObjectAnimator.ofFloat(target, "scaleY", srcScaleY, destScaleY)
        );
        animator.start();
    }

    private void initIcon() {
        lp = new WindowManager.LayoutParams();
        lp.width = mFingerprintImageSize.width;
        lp.height = mFingerprintImageSize.height;
        lp.y = mFingerprintImageYCenter - mFingerprintImageSize.height / 2;
        lp.x = mFingerprintImageXCenter - mFingerprintImageSize.width / 2;
        lp.gravity = Gravity.BOTTOM | Gravity.LEFT;
        lp.format = PixelFormat.TRANSPARENT;
        lp.type = WindowManager.LayoutParams.TYPE_VOLUME_OVERLAY;
        lp.windowAnimations = -1;
        lp.screenBrightness = 1;
        lp.flags = lp.flags
                | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                | WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH
                | WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                | WindowManager.LayoutParams.FLAG_FULLSCREEN;

        mSensorImgView = new SensorImageView(getApplicationContext());
        mSensorImgView.setScaleType(ImageView.ScaleType.CENTER_CROP);
    }

    private void showIcon() {
        if (mSensorImgView != null) {
            WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
            wm.addView(mSensorImgView, lp);
        }
    }

    private void hideIcon() {
        if (mSensorImgView != null) {
            WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
            wm.removeView(mSensorImgView);
        }
    }

    public class SensorImageView extends ImageView {
        public static final int SEARCH_ICON_FLAG_FINGER_ICON = 1;
        public static final int SEARCH_ICON_FLAG_GRID = 2;
        public static final int SEARCH_ICON_FLAG_DRAG = 3;

        private int y_down;
        private int x_down;

        private int mFunStep = 0;
        private boolean mZoom = false;
        private boolean mDrawFingerIcon = false;
        private boolean mDrawGrid = true;
        private boolean mDragEnable = true;
        private final int mCircelCount = 4;
        private final int mCenterRadius = 8;
        private final int mLineWidth = 2;

        private Paint mGridLinePaint;
        private Paint mGridCirclePaint;

        public SensorImageView(Context context) {
            this(context, null);
        }

        public SensorImageView(Context context, AttributeSet attrs) {
            this(context, attrs, 0);
        }

        public SensorImageView(Context context, AttributeSet attrs, int defStyleAttr) {
            super(context, attrs, defStyleAttr);

            mGridLinePaint = new Paint();
            mGridLinePaint.setColor(Color.BLACK);
            mGridLinePaint.setAntiAlias(true);
            mGridLinePaint.setStrokeWidth(mLineWidth);
            mGridLinePaint.setStyle(Paint.Style.FILL_AND_STROKE);

            mGridCirclePaint = new Paint();
            mGridCirclePaint.setColor(Color.BLACK);
            mGridCirclePaint.setAntiAlias(true);
            mGridCirclePaint.setStrokeWidth(mLineWidth);
            mGridCirclePaint.setStyle(Paint.Style.STROKE);

            updateIcon(context);
        }

        private void updateIcon(Context context) {
            if (mDrawFingerIcon) {
                setImageDrawable(context.getDrawable(mFingerprintImageId));
            } else {
                setImageDrawable(null);
            }
        }

        public void updateFun(Context context, int type) {
            switch (type) {
                case SEARCH_ICON_FLAG_FINGER_ICON: {
                    mDrawFingerIcon = !mDrawFingerIcon;
                    updateIcon(context);
                    break;
                }
                case SEARCH_ICON_FLAG_GRID: {
                    mDrawGrid = !mDrawGrid;
                    invalidate();
                    break;
                }
                case SEARCH_ICON_FLAG_DRAG: {
                    mDragEnable = !mDragEnable;
                    break;
                }
            }
            mFunStep++;
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN: {
                    x_down = (int) event.getRawX();
                    y_down = (int) event.getRawY();
                    if (mZoom) {
                        zoomView(this, 1f, 1.5f, 1f, 1.5f);
                    }
                    break;
                }
                case MotionEvent.ACTION_MOVE: {
                    int x = (int) event.getRawX();
                    int y = (int) event.getRawY();

                    if (mDragEnable) {
                        updatePosition(x - x_down , y_down - y);
                    }

                    x_down = x;
                    y_down = y;
                    break;
                }
                case MotionEvent.ACTION_UP: {
                    if (mZoom) {
                        zoomView(this, 1.5f, 1f, 1.5f, 1f);
                    }
                    break;
                }
            }

            return true;
        }

        @Override
        protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);

            if (mDrawGrid) {
                int width = getMeasuredWidth();
                int height = getMeasuredHeight();
                int lineRadius = mLineWidth / 2;

                canvas.drawLine(lineRadius, lineRadius, lineRadius, height - lineRadius, mGridLinePaint);
                canvas.drawLine(width - lineRadius, lineRadius, width-lineRadius, height-lineRadius, mGridLinePaint);
                canvas.drawLine(lineRadius, lineRadius, width-lineRadius, lineRadius, mGridLinePaint);
                canvas.drawLine(lineRadius, height-lineRadius, width-lineRadius, height-lineRadius, mGridLinePaint);

                canvas.drawLine(0, height/2, width, height/2, mGridLinePaint);
                canvas.drawLine(width/2, 0, width/2, height, mGridLinePaint);
                canvas.drawCircle(width/2, height/2, mCenterRadius, mGridLinePaint);

                canvas.drawCircle(width/2, height/2, width/2 - lineRadius, mGridCirclePaint);

                int radius = 0;
                for (int i = 1; i < mCircelCount; i++) {
                    radius = (width * (mCircelCount - i)) / (mCircelCount * 2);
                    canvas.drawLine(radius, 0, radius, height, mGridLinePaint);
                    canvas.drawLine(radius + width/2, 0, radius + width/2, height, mGridLinePaint);
                    canvas.drawCircle(width/2, height/2, radius, mGridCirclePaint);
                }
            }
        }
    }
}