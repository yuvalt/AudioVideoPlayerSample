package com.serenegiant.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.util.AttributeSet;
import android.view.View;

public class OverlayView extends View {
    final private Paint paint = new Paint();
    private Point[] points;

    public OverlayView(Context context, AttributeSet attrs) {
        super(context, attrs);
        points = new Point[0];
        paint.setColor(Color.BLUE);
    }

    public void drawPoints(Point[] points) {
        this.points = points;
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {

        for (int i = 0; i < points.length; i++) {
            //			Log.v(TAG, Float.toString(points[i].x) + "," + Float.toString(points[i].y));
            canvas.drawCircle(points[i].x, points[i].y, 10, paint);
        }
    }
}
